#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "logger.h"

static FILE *server_log_file = NULL;
static FILE *transaction_log_file = NULL;

static pthread_mutex_t log_mutex;


/* Get current date and time */
static void get_timestamp(char *buffer, size_t size)
{
    time_t current_time;
    struct tm *time_info;

    current_time = time(NULL);
    time_info = localtime(&current_time);

    if (time_info == NULL) {
        snprintf(buffer, size, "UNKNOWN-TIME");
        return;
    }

    strftime(
        buffer,
        size,
        "%Y-%m-%d %H:%M:%S",
        time_info
    );
}


/* Initialize logger */
int logger_init(void)
{
    server_log_file = fopen("logs/server.log", "a");
    if (server_log_file == NULL) {
        perror("Unable to open server log");
        return -1;
    }

    transaction_log_file = fopen("logs/transaction.log", "a");
    if (transaction_log_file == NULL) {
        perror("Unable to open transaction log");
        fclose(server_log_file);
        server_log_file = NULL;
        return -1;
    }

    if (pthread_mutex_init(&log_mutex, NULL) != 0) {
        perror("Unable to initialize logger mutex");
        fclose(server_log_file);
        fclose(transaction_log_file);

        server_log_file = NULL;
        transaction_log_file = NULL;

        return -1;
    }

    log_server_activity("Logger initialized successfully");

    return 0;
}


/* Close logger */
void logger_close(void)
{
    pthread_mutex_lock(&log_mutex);

    if (server_log_file != NULL) {
        fclose(server_log_file);
        server_log_file = NULL;
    }

    if (transaction_log_file != NULL) {
        fclose(transaction_log_file);
        transaction_log_file = NULL;
    }

    pthread_mutex_unlock(&log_mutex);

    pthread_mutex_destroy(&log_mutex);
}


/* Write server activity log */
void log_server_activity(const char *message)
{
    char timestamp[64];

    if (server_log_file == NULL || message == NULL) {
        return;
    }

    get_timestamp(timestamp, sizeof(timestamp));

    pthread_mutex_lock(&log_mutex);

    fprintf(
        server_log_file,
        "[%s] SERVER: %s\n",
        timestamp,
        message
    );

    fflush(server_log_file);

    pthread_mutex_unlock(&log_mutex);
}


/* Write transaction log */
void log_transaction(
    const char *operation,
    int student_id,
    const char *details
)
{
    char timestamp[64];

    if (transaction_log_file == NULL ||
        operation == NULL ||
        details == NULL) {
        return;
    }

    get_timestamp(timestamp, sizeof(timestamp));

    pthread_mutex_lock(&log_mutex);

    fprintf(
        transaction_log_file,
        "[%s] OPERATION: %s | STUDENT_ID: %d | DETAILS: %s\n",
        timestamp,
        operation,
        student_id,
        details
    );

    fflush(transaction_log_file);

    pthread_mutex_unlock(&log_mutex);
}
