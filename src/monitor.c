#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "../include/monitor.h"

void monitor_process(void)
{
    char path[100];
    char line[256];

    snprintf(path, sizeof(path), "/proc/%d/status", getpid());

    FILE *file = fopen(path, "r");

    if (file == NULL) {
        perror("Unable to open process status");
        return;
    }

    printf("\n========== SERVER MONITOR ==========\n");
    printf("Server PID: %d\n", getpid());

    while (fgets(line, sizeof(line), file) != NULL) {

        if (strncmp(line, "VmRSS:", 6) == 0 ||
            strncmp(line, "Threads:", 8) == 0) {

            printf("%s", line);
        }
    }

    printf("====================================\n");

    fclose(file);
}


/* Get memory usage in KB */
int get_memory_usage_kb(void)
{
    char path[100];
    char line[256];
    int memory = 0;

    snprintf(path, sizeof(path), "/proc/%d/status", getpid());

    FILE *file = fopen(path, "r");

    if (file == NULL)
        return 0;

    while (fgets(line, sizeof(line), file) != NULL) {

        if (strncmp(line, "VmRSS:", 6) == 0) {

            sscanf(line, "VmRSS: %d", &memory);
            break;
        }
    }

    fclose(file);

    return memory;
}


/* Get total Linux threads */
int get_thread_count(void)
{
    char path[100];
    char line[256];
    int threads = 0;

    snprintf(path, sizeof(path), "/proc/%d/status", getpid());

    FILE *file = fopen(path, "r");

    if (file == NULL)
        return 0;

    while (fgets(line, sizeof(line), file) != NULL) {

        if (strncmp(line, "Threads:", 8) == 0) {

            sscanf(line, "Threads: %d", &threads);
            break;
        }
    }

    fclose(file);

    return threads;
}
