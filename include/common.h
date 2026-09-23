#ifndef COMMON_H
#define COMMON_H

#define DEFAULT_PORT 8080

#define MAX_REQUEST 512
#define MAX_RESPONSE 2048

#define QUEUE_SIZE 50
#define MAX_WORKERS 4
#define MAX_STUDENTS 100

#define MAX_CLIENTS 20

typedef struct {
    int id;
    char name[50];
    char course[30];
    float marks;
    float attendance;
    int active;
} Student;

typedef struct {
    int client_fd;
    char request[MAX_REQUEST];
} Request;

typedef struct {
    int client_fd;
    char response[MAX_RESPONSE];
} Response;

#endif
