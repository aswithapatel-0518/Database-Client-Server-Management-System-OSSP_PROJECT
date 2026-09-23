#include <stdio.h>

#include "logger.h"

int main(void)
{
    printf("Initializing logger...\n");

    if (logger_init() != 0) {
        printf("Logger initialization failed.\n");
        return 1;
    }

    printf("Logger initialized successfully.\n");

    log_server_activity("Test server started");
    log_server_activity("Test client connected");

    log_transaction(
        "INSERT",
        101,
        "Student inserted successfully"
    );

    log_transaction(
        "UPDATE",
        101,
        "Student marks updated"
    );

    log_transaction(
        "DELETE",
        101,
        "Student deleted successfully"
    );

    logger_close();

    printf("Log entries written successfully.\n");

    return 0;
}
