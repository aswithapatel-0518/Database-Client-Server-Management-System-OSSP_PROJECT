#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define API_PORT 8081
#define SERVER_PORT 8080
#define BUFFER_SIZE 4096

void send_http_response(int client_fd, const char *body)
{
    char response[BUFFER_SIZE + 512];

    int body_length = strlen(body);

    snprintf(
        response,
        sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        body_length,
        body
    );

    send(client_fd, response, strlen(response), 0);
}


/* Convert URL encoding back to normal text */
void url_decode(char *src, char *dest, size_t dest_size)
{
    size_t i = 0;
    size_t j = 0;

    while (src[i] != '\0' && j < dest_size - 1) {

        if (src[i] == '%') {

            if (src[i + 1] != '\0' &&
                src[i + 2] != '\0') {

                char hex[3];

                hex[0] = src[i + 1];
                hex[1] = src[i + 2];
                hex[2] = '\0';

                dest[j++] = (char)strtol(hex, NULL, 16);

                i += 3;
            } else {
                break;
            }

        } else if (src[i] == '+') {

            dest[j++] = ' ';
            i++;

        } else {

            dest[j++] = src[i];
            i++;
        }
    }

    dest[j] = '\0';
}


/* Connect to C server */
int connect_to_server()
{
    int sock;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
        return -1;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &server_addr.sin_addr
    );

    if (connect(
            sock,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)
        ) < 0) {

        close(sock);
        return -1;
    }

    return sock;
}


/* Send command to C server and receive response */
int send_command(
    int server_fd,
    const char *command,
    char *response
)
{
    memset(response, 0, BUFFER_SIZE);

    send(
        server_fd,
        command,
        strlen(command),
        0
    );

    int bytes = recv(
        server_fd,
        response,
        BUFFER_SIZE - 1,
        0
    );

    if (bytes <= 0)
        return -1;

    response[bytes] = '\0';

    return 0;
}


void handle_request(int browser_fd)
{
    char request[BUFFER_SIZE];

    char command_encoded[BUFFER_SIZE];
    char command[BUFFER_SIZE];

    char username[100];
    char password[100];

    char response[BUFFER_SIZE];
    char login_response[BUFFER_SIZE];

    memset(request, 0, sizeof(request));
    memset(command_encoded, 0, sizeof(command_encoded));
    memset(command, 0, sizeof(command));

    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));

    recv(
        browser_fd,
        request,
        sizeof(request) - 1,
        0
    );


    /*
     * Extract command
     */

    char *command_start =
        strstr(request, "command=");

    if (command_start == NULL) {

        send_http_response(
            browser_fd,
            "ERROR: No command received"
        );

        return;
    }

    command_start += strlen("command=");

    char *command_end =
        strchr(command_start, '&');

    if (command_end == NULL)
        command_end = strchr(command_start, ' ');

    if (command_end != NULL) {

        size_t length =
            command_end - command_start;

        if (length >= sizeof(command_encoded))
            length = sizeof(command_encoded) - 1;

        strncpy(
            command_encoded,
            command_start,
            length
        );

        command_encoded[length] = '\0';

    } else {

        strncpy(
            command_encoded,
            command_start,
            sizeof(command_encoded) - 1
        );
    }


    /*
     * Decode command
     */

    url_decode(
        command_encoded,
        command,
        sizeof(command)
    );


    /*
     * Extract username
     */

    char *username_start =
        strstr(request, "username=");

    if (username_start != NULL) {

        username_start += strlen("username=");

        char *username_end =
            strchr(username_start, '&');

        if (username_end != NULL) {

            size_t length =
                username_end - username_start;

            if (length >= sizeof(username))
                length = sizeof(username) - 1;

            strncpy(
                username,
                username_start,
                length
            );

            username[length] = '\0';

        } else {

            strncpy(
                username,
                username_start,
                sizeof(username) - 1
            );
        }
    }


    /*
     * Extract password
     */

    char *password_start =
        strstr(request, "password=");

    if (password_start != NULL) {

        password_start += strlen("password=");

        char *password_end =
            strchr(password_start, '&');

        if (password_end != NULL) {

            size_t length =
                password_end - password_start;

            if (length >= sizeof(password))
                length = sizeof(password) - 1;

            strncpy(
                password,
                password_start,
                length
            );

            password[length] = '\0';

        } else {

            strncpy(
                password,
                password_start,
                sizeof(password) - 1
            );
        }
    }


    /*
     * Decode username and password
     */

    char decoded_username[100];
    char decoded_password[100];

    url_decode(
        username,
        decoded_username,
        sizeof(decoded_username)
    );

    url_decode(
        password,
        decoded_password,
        sizeof(decoded_password)
    );


    /*
     * CONNECT TO C SERVER
     */

    int server_fd = connect_to_server();

    if (server_fd < 0) {

        send_http_response(
            browser_fd,
            "ERROR: Cannot connect to C server"
        );

        return;
    }


    /*
     * REGISTER
     *
     * Registration does NOT require login.
     */

    if (strcmp(command, "REGISTER") == 0) {

        char register_command[300];

        snprintf(
            register_command,
            sizeof(register_command),
            "REGISTER %s %s",
            decoded_username,
            decoded_password
        );

        if (send_command(
                server_fd,
                register_command,
                response
            ) < 0) {

            close(server_fd);

            send_http_response(
                browser_fd,
                "ERROR: No response from server"
            );

            return;
        }

        close(server_fd);

        send_http_response(
            browser_fd,
            response
        );

        return;
    }


    /*
     * LOGIN
     */

    if (strcmp(command, "LOGIN") == 0) {

        char login_command[300];

        snprintf(
            login_command,
            sizeof(login_command),
            "LOGIN %s %s",
            decoded_username,
            decoded_password
        );

        if (send_command(
                server_fd,
                login_command,
                response
            ) < 0) {

            close(server_fd);

            send_http_response(
                browser_fd,
                "ERROR: No response from server"
            );

            return;
        }

        close(server_fd);

        send_http_response(
            browser_fd,
            response
        );

        return;
    }


    /*
     * For all other commands:
     *
     * 1. LOGIN
     * 2. Execute requested command
     */

    char login_command[300];

    snprintf(
        login_command,
        sizeof(login_command),
        "LOGIN %s %s",
        decoded_username,
        decoded_password
    );


    if (send_command(
            server_fd,
            login_command,
            login_response
        ) < 0) {

        close(server_fd);

        send_http_response(
            browser_fd,
            "ERROR: Login communication failed"
        );

        return;
    }


    /*
     * Check whether login succeeded
     */

    if (strncmp(
            login_response,
            "SUCCESS",
            7
        ) != 0) {

        close(server_fd);

        send_http_response(
            browser_fd,
            login_response
        );

        return;
    }


    /*
     * Send actual database command
     */

    if (send_command(
            server_fd,
            command,
            response
        ) < 0) {

        close(server_fd);

        send_http_response(
            browser_fd,
            "ERROR: Database server communication failed"
        );

        return;
    }


    close(server_fd);

    send_http_response(
        browser_fd,
        response
    );
}


int main()
{
    int server_fd;
    int browser_fd;

    struct sockaddr_in server_addr;
    struct sockaddr_in browser_addr;

    socklen_t browser_len =
        sizeof(browser_addr);

    server_fd =
        socket(
            AF_INET,
            SOCK_STREAM,
            0
        );

    if (server_fd < 0) {

        perror("socket");

        return 1;
    }


    int opt = 1;

    setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );


    memset(
        &server_addr,
        0,
        sizeof(server_addr)
    );

    server_addr.sin_family =
        AF_INET;

    server_addr.sin_addr.s_addr =
        inet_addr("127.0.0.1");

    server_addr.sin_port =
        htons(API_PORT);


    if (bind(
            server_fd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)
        ) < 0) {

        perror("bind");

        close(server_fd);

        return 1;
    }


    if (listen(server_fd, 10) < 0) {

        perror("listen");

        close(server_fd);

        return 1;
    }


    printf(
        "Dashboard API running on port %d...\n",
        API_PORT
    );


    while (1) {

        browser_fd =
            accept(
                server_fd,
                (struct sockaddr *)&browser_addr,
                &browser_len
            );

        if (browser_fd < 0) {

            perror("accept");

            continue;
        }


        handle_request(browser_fd);

        close(browser_fd);
    }


    close(server_fd);

    return 0;
}
