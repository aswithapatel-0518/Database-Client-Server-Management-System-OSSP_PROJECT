#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../include/common.h"
#include "../include/config.h"
#include "../include/database.h"
#include "../include/queue.h"
#include "../include/logger.h"
#include "../include/monitor.h"
static volatile sig_atomic_t server_running = 1;
static int server_fd = -1;

/* Handle SIGINT and SIGTERM */
void handle_shutdown(int signal_number)
{
    (void)signal_number;

    server_running = 0;

    if (server_fd != -1) {
        close(server_fd);
        server_fd = -1;
    }
}

/* Handle SIGPIPE */
void handle_sigpipe(int signal_number)
{
    (void)signal_number;
}

/* Process one database request */
void process_request(Request request, Response *response)
{
    char command[32];

    response->client_fd = request.client_fd;
    response->response[0] = '\0';

    if (sscanf(request.request, "%31s", command) != 1) {
        snprintf(
            response->response,
            MAX_RESPONSE,
            "ERROR: Empty request\n"
        );
        return;
    }

    /* INSERT command */
    if (strcmp(command, "INSERT") == 0) {
        Student student;

        int fields = sscanf(
            request.request,
            "INSERT %d %49s %29s %f %f",
            &student.id,
            student.name,
            student.course,
            &student.marks,
            &student.attendance
        );

        if (fields != 5) {
            snprintf(
                response->response,
                MAX_RESPONSE,
                "ERROR: Usage: INSERT id name course marks attendance\n"
            );
            return;
        }

        student.active = 1;

        if (insert_student(student) == 0) {
            snprintf(
                response->response,
                MAX_RESPONSE,
                "SUCCESS: Student inserted\n"
            );
        } else {
            snprintf(
                response->response,
                MAX_RESPONSE,
                "ERROR: Student insertion failed\n"
            );
        }
    }

    /* SEARCH command */
    else if (strcmp(command, "SEARCH") == 0) {
        int id;
        Student student;

        if (sscanf(request.request, "SEARCH %d", &id) != 1) {
            snprintf(
                response->response,
                MAX_RESPONSE,
                "ERROR: Usage: SEARCH id\n"
            );
            return;
        }

        if (search_student(id, &student) == 0) {
            snprintf(
                response->response,
                MAX_RESPONSE,
                "SUCCESS: ID=%d Name=%s Course=%s Marks=%.2f Attendance=%.2f\n",
                student.id,
                student.name,
                student.course,
                student.marks,
                student.attendance
            );
        } else {
            snprintf(
                response->response,
                MAX_RESPONSE,
                "ERROR: Student not found\n"
            );
        }
    }

    /* UPDATE command */
    else if (strcmp(command, "UPDATE") == 0) {
        int id;
        float marks;
        float attendance;

        int fields = sscanf(
            request.request,
            "UPDATE %d %f %f",
            &id,
            &marks,
            &attendance
        );

        if (fields != 3) {
            snprintf(
                response->response,
                MAX_RESPONSE,
                "ERROR: Usage: UPDATE id marks attendance\n"
            );
            return;
        }

        if (update_student(id, marks, attendance) == 0) {
            snprintf(
                response->response,
                MAX_RESPONSE,
                "SUCCESS: Student updated\n"
            );
        } else {
            snprintf(
                response->response,
                MAX_RESPONSE,
                "ERROR: Student update failed or student not found\n"
            );
        }
    }

    /* DELETE command */
    else if (strcmp(command, "DELETE") == 0) {
        int id;

        if (sscanf(request.request, "DELETE %d", &id) != 1) {
            snprintf(
                response->response,
                MAX_RESPONSE,
                "ERROR: Usage: DELETE id\n"
            );
            return;
        }

        if (delete_student(id) == 0) {
            snprintf(
                response->response,
                MAX_RESPONSE,
                "SUCCESS: Student deleted\n"
            );
        } else {
            snprintf(
                response->response,
                MAX_RESPONSE,
                "ERROR: Student deletion failed or student not found\n"
            );
        }
    }

    /* DISPLAY command */
    else if (strcmp(command, "DISPLAY") == 0) {
        display_all_students();

        snprintf(
            response->response,
            MAX_RESPONSE,
            "SUCCESS: Student records displayed on server terminal\n"
        );
    }
/* DISPLAY command */
else if (strcmp(command, "DISPLAY") == 0) {
    display_all_students();

    snprintf(
        response->response,
        MAX_RESPONSE,
        "SUCCESS: Student records displayed on server terminal\n"
    );
}

/* MONITOR command */
else if (strcmp(command, "MONITOR") == 0) {
    monitor_process();

    snprintf(
        response->response,
        MAX_RESPONSE,
        "SUCCESS: Monitoring information displayed on server terminal\n"
    );
}

/* EXIT command */
else if (strcmp(command, "EXIT") == 0) {
    snprintf(
        response->response,
        MAX_RESPONSE,
        "SUCCESS: Connection closing\n"
    );
}

    /* EXIT command */
    else if (strcmp(command, "EXIT") == 0) {
        snprintf(
            response->response,
            MAX_RESPONSE,
            "SUCCESS: Connection closing\n"
        );
    }

    /* Invalid command */
    else {
        snprintf(
            response->response,
            MAX_RESPONSE,
            "ERROR: Unknown command\n"
            "Available commands: INSERT, SEARCH, UPDATE, DELETE, DISPLAY, MONITOR, EXIT\n"
        );
    }
}

/* Worker thread function */
void *worker_thread(void *arg)
{
    long worker_id = (long)arg;

    char log_message[100];
    snprintf(
        log_message,
        sizeof(log_message),
        "Worker thread %ld started",
        worker_id
    );
    log_server_activity(log_message);

    while (server_running) {
        Request request;
        Response response;

        if (dequeue_request(&request) != 0) {
            continue;
        }

        process_request(request, &response);

        if (enqueue_response(response) != 0) {
            log_server_activity("Failed to enqueue response");
        }
    }

    snprintf(
        log_message,
        sizeof(log_message),
        "Worker thread %ld stopped",
        worker_id
    );
    log_server_activity(log_message);

    return NULL;
}

/* Response thread function */
void *response_thread(void *arg)
{
    (void)arg;

    while (server_running) {
        Response response;

        if (dequeue_response(&response) != 0) {
            continue;
        }

        if (send(
                response.client_fd,
                response.response,
                strlen(response.response),
                0
            ) == -1) {
            perror("Response send failed");
        }

        /*
         * The client sends one request at a time and receives one response.
         * The client connection is closed by the client after EXIT.
         */
    }

    return NULL;
}

/* Receive requests from one client */
void *client_thread(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);

    char client_ip[INET_ADDRSTRLEN] = "unknown";
    struct sockaddr_in client_address;
    socklen_t address_length = sizeof(client_address);

    if (getpeername(
            client_fd,
            (struct sockaddr *)&client_address,
            &address_length
        ) == 0) {
        inet_ntop(
            AF_INET,
            &client_address.sin_addr,
            client_ip,
            sizeof(client_ip)
        );
    }

    char log_message[200];
    snprintf(
        log_message,
        sizeof(log_message),
        "Client connected: %s",
        client_ip
    );
    log_server_activity(log_message);

    while (server_running) {
        char buffer[MAX_REQUEST];
        memset(buffer, 0, sizeof(buffer));

        ssize_t bytes_received = recv(
            client_fd,
            buffer,
            sizeof(buffer) - 1,
            0
        );

        if (bytes_received == 0) {
            break;
        }

        if (bytes_received < 0) {
            if (errno == EINTR) {
                continue;
            }

            perror("Client receive failed");
            break;
        }

        buffer[bytes_received] = '\0';

        Request request;
        request.client_fd = client_fd;

        strncpy(
            request.request,
            buffer,
            MAX_REQUEST - 1
        );
        request.request[MAX_REQUEST - 1] = '\0';

        if (enqueue_request(request) != 0) {
            Response error_response;
            error_response.client_fd = client_fd;

            snprintf(
                error_response.response,
                MAX_RESPONSE,
                "ERROR: Request queue is full\n"
            );

            send(
                client_fd,
                error_response.response,
                strlen(error_response.response),
                0
            );
        }

        if (strncmp(buffer, "EXIT", 4) == 0) {
            break;
        }
    }

    close(client_fd);

    snprintf(
        log_message,
        sizeof(log_message),
        "Client disconnected: %s",
        client_ip
    );
    log_server_activity(log_message);

    return NULL;
}

int main(void)
{
    struct sockaddr_in server_address;

    pthread_t workers[MAX_WORKERS];
    pthread_t response_tid;

    /* Load configuration */
    if (load_config() != 0) {
        fprintf(stderr, "Configuration loading failed\n");
        return EXIT_FAILURE;
    }

    print_config();

    /* Initialize database */
    if (database_init() != 0) {
        fprintf(stderr, "Database initialization failed\n");
        return EXIT_FAILURE;
    }

    /* Initialize request queue */
    if (request_queue_init() != 0) {
        fprintf(stderr, "Request queue initialization failed\n");
        database_close();
        return EXIT_FAILURE;
    }

    /* Initialize response queue */
    if (response_queue_init() != 0) {
        fprintf(stderr, "Response queue initialization failed\n");
        request_queue_destroy();
        database_close();
        return EXIT_FAILURE;
    }

    /* Initialize logger */
    if (logger_init() != 0) {
        fprintf(stderr, "Logger initialization failed\n");
        response_queue_destroy();
        request_queue_destroy();
        database_close();
        return EXIT_FAILURE;
    }

    /* Register signal handlers */
    signal(SIGINT, handle_shutdown);
    signal(SIGTERM, handle_shutdown);
    signal(SIGPIPE, handle_sigpipe);

    /* Create server socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("Socket creation failed");
        logger_close();
        response_queue_destroy();
        request_queue_destroy();
        database_close();
        return EXIT_FAILURE;
    }

    int reuse = 1;

    if (setsockopt(
            server_fd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuse,
            sizeof(reuse)
        ) == -1) {
        perror("setsockopt failed");
        close(server_fd);
        logger_close();
        response_queue_destroy();
        request_queue_destroy();
        database_close();
        return EXIT_FAILURE;
    }

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(server_config.port);

    /* Bind socket */
    if (bind(
            server_fd,
            (struct sockaddr *)&server_address,
            sizeof(server_address)
        ) == -1) {
        perror("Bind failed");
        close(server_fd);
        logger_close();
        response_queue_destroy();
        request_queue_destroy();
        database_close();
        return EXIT_FAILURE;
    }

    /* Listen for clients */
    if (listen(server_fd, server_config.max_clients) == -1) {
        perror("Listen failed");
        close(server_fd);
        logger_close();
        response_queue_destroy();
        request_queue_destroy();
        database_close();
        return EXIT_FAILURE;
    }

    printf("\n========================================\n");
    printf(" Database Client-Server Management System\n");
    printf("========================================\n");
    printf("Server running on port %d\n", server_config.port);
    printf("Maximum clients: %d\n", server_config.max_clients);
    printf("Worker threads: %d\n", server_config.workers);
    printf("Waiting for client connections...\n\n");

    log_server_activity("Server started successfully");

    /* Create worker threads */
    int worker_count = server_config.workers;

    if (worker_count > MAX_WORKERS) {
        worker_count = MAX_WORKERS;
    }

    for (int i = 0; i < worker_count; i++) {
        if (pthread_create(
                &workers[i],
                NULL,
                worker_thread,
                (void *)(long)(i + 1)
            ) != 0) {
            perror("Worker thread creation failed");
            server_running = 0;
            break;
        }
    }

    /* Create response thread */
    if (server_running) {
        if (pthread_create(
                &response_tid,
                NULL,
                response_thread,
                NULL
            ) != 0) {
            perror("Response thread creation failed");
            server_running = 0;
        }
    }

    /* Accept client connections */
    while (server_running) {
        struct sockaddr_in client_address;
        socklen_t client_length = sizeof(client_address);

        int client_fd = accept(
            server_fd,
            (struct sockaddr *)&client_address,
            &client_length
        );

        if (client_fd == -1) {
            if (!server_running) {
                break;
            }

            if (errno == EINTR) {
                continue;
            }

            perror("Accept failed");
            continue;
        }

        int *client_fd_ptr = malloc(sizeof(int));

        if (client_fd_ptr == NULL) {
            perror("Memory allocation failed");
            close(client_fd);
            continue;
        }

        *client_fd_ptr = client_fd;

        pthread_t client_tid;

        if (pthread_create(
                &client_tid,
                NULL,
                client_thread,
                client_fd_ptr
            ) != 0) {
            perror("Client thread creation failed");
            close(client_fd);
            free(client_fd_ptr);
            continue;
        }

        /*
         * Client threads handle their own connections.
         * Detaching avoids the need to join every client thread.
         */
        pthread_detach(client_tid);
    }

    printf("\nShutting down server...\n");
    log_server_activity("Server shutdown initiated");

    server_running = 0;

    if (server_fd != -1) {
        close(server_fd);
        server_fd = -1;
    }

    /* Cancel and join worker threads */
    for (int i = 0; i < worker_count; i++) {
        pthread_cancel(workers[i]);
        pthread_join(workers[i], NULL);
    }

    /* Cancel and join response thread */
    pthread_cancel(response_tid);
    pthread_join(response_tid, NULL);

    /* Release resources */
    response_queue_destroy();
    request_queue_destroy();
    database_close();
    logger_close();

    printf("Server stopped successfully.\n");

    return EXIT_SUCCESS;
}
