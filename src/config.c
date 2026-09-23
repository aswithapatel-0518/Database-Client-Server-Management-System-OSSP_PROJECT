#include <stdio.h>
#include <string.h>

#include "../include/config.h"

Config server_config;

int load_config(void)
{
    FILE *file = fopen("config.txt", "r");

    if (file == NULL) {
        perror("Unable to open config.txt");
        return -1;
    }

    char line[100];

    while (fgets(line, sizeof(line), file)) {

        if (strncmp(line, "PORT=", 5) == 0) {
            sscanf(line + 5, "%d", &server_config.port);
        }
        else if (strncmp(line, "WORKERS=", 8) == 0) {
            sscanf(line + 8, "%d", &server_config.workers);
        }
        else if (strncmp(line, "MAX_CLIENTS=", 12) == 0) {
            sscanf(line + 12, "%d", &server_config.max_clients);
        }
        else if (strncmp(line, "LOG_LEVEL=", 10) == 0) {
            sscanf(line + 10, "%d", &server_config.log_level);
        }
    }

    fclose(file);

    return 0;
}

void print_config(void)
{
    printf("\n========== SERVER CONFIGURATION ==========\n");
    printf("Port         : %d\n", server_config.port);
    printf("Workers      : %d\n", server_config.workers);
    printf("Max Clients  : %d\n", server_config.max_clients);
    printf("Log Level    : %d\n", server_config.log_level);
    printf("===========================================\n");
}
