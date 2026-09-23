#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define MAX_REQUEST 512
#define MAX_RESPONSE 2048

int main(void)
{
    int client_fd;
    struct sockaddr_in server_address;

    char request[MAX_REQUEST];
    char response[MAX_RESPONSE];

    /* Create socket */
    client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_fd == -1) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    if (inet_pton(
            AF_INET,
            SERVER_IP,
            &server_address.sin_addr
        ) <= 0) {
        perror("Invalid server address");
        close(client_fd);
        return EXIT_FAILURE;
    }

    /* Connect to server */
    if (connect(
            client_fd,
            (struct sockaddr *)&server_address,
            sizeof(server_address)
        ) == -1) {
        perror("Connection to server failed");
        close(client_fd);
        return EXIT_FAILURE;
    }

    printf("Connected to Database Server!\n");
    printf("Server IP: %s\n", SERVER_IP);
    printf("Server Port: %d\n", SERVER_PORT);

    printf("\nAvailable commands:\n");
    printf("INSERT id name course marks attendance\n");
    printf("SEARCH id\n");
    printf("UPDATE id marks attendance\n");
    printf("DELETE id\n");
    printf("DISPLAY\n");
    printf("MONITOR\n");
    printf("EXIT\n");

    while (1) {
        printf("\nEnter request: ");
        fflush(stdout);

        if (fgets(request, sizeof(request), stdin) == NULL) {
            break;
        }

        request[strcspn(request, "\n")] = '\0';

        if (strlen(request) == 0) {
            printf("Request cannot be empty.\n");
            continue;
        }

        /* Send request to server */
        if (send(
                client_fd,
                request,
                strlen(request),
                0
            ) == -1) {
            perror("Request send failed");
            break;
        }

        /* Receive response from server */
        memset(response, 0, sizeof(response));

        ssize_t bytes_received = recv(
            client_fd,
            response,
            sizeof(response) - 1,
            0
        );

        if (bytes_received <= 0) {
            printf("Server disconnected.\n");
            break;
        }

        response[bytes_received] = '\0';

        printf("Server response: %s", response);

        if (strcmp(request, "EXIT") == 0) {
            break;
        }
    }

    close(client_fd);

    printf("\nDisconnected from server.\n");

    return EXIT_SUCCESS;
}
