#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include "queue.h"

/* ================= REQUEST QUEUE ================= */

static Request request_queue[QUEUE_SIZE];

static int request_front = 0;
static int request_rear = 0;

static pthread_mutex_t request_mutex;
static sem_t request_empty;
static sem_t request_full;

/* ================= RESPONSE QUEUE ================= */

static Response response_queue[QUEUE_SIZE];

static int response_front = 0;
static int response_rear = 0;

static pthread_mutex_t response_mutex;
static sem_t response_empty;
static sem_t response_full;


/* Initialize request queue */
int request_queue_init(void)
{
    request_front = 0;
    request_rear = 0;

    if (pthread_mutex_init(&request_mutex, NULL) != 0) {
        perror("Request mutex initialization failed");
        return -1;
    }

    if (sem_init(&request_empty, 0, QUEUE_SIZE) != 0) {
        perror("Request empty semaphore initialization failed");
        pthread_mutex_destroy(&request_mutex);
        return -1;
    }

    if (sem_init(&request_full, 0, 0) != 0) {
        perror("Request full semaphore initialization failed");
        sem_destroy(&request_empty);
        pthread_mutex_destroy(&request_mutex);
        return -1;
    }

    return 0;
}


/* Destroy request queue */
void request_queue_destroy(void)
{
    sem_destroy(&request_empty);
    sem_destroy(&request_full);
    pthread_mutex_destroy(&request_mutex);
}


/* Add request to request queue */
int enqueue_request(Request request)
{
    if (sem_wait(&request_empty) != 0) {
        perror("Request empty semaphore wait failed");
        return -1;
    }

    pthread_mutex_lock(&request_mutex);

    request_queue[request_rear] = request;
    request_rear = (request_rear + 1) % QUEUE_SIZE;

    pthread_mutex_unlock(&request_mutex);

    if (sem_post(&request_full) != 0) {
        perror("Request full semaphore post failed");
        return -1;
    }

    return 0;
}


/* Remove request from request queue */
int dequeue_request(Request *request)
{
    if (request == NULL) {
        return -1;
    }

    if (sem_wait(&request_full) != 0) {
        perror("Request full semaphore wait failed");
        return -1;
    }

    pthread_mutex_lock(&request_mutex);

    *request = request_queue[request_front];
    request_front = (request_front + 1) % QUEUE_SIZE;

    pthread_mutex_unlock(&request_mutex);

    if (sem_post(&request_empty) != 0) {
        perror("Request empty semaphore post failed");
        return -1;
    }

    return 0;
}


/* Initialize response queue */
int response_queue_init(void)
{
    response_front = 0;
    response_rear = 0;

    if (pthread_mutex_init(&response_mutex, NULL) != 0) {
        perror("Response mutex initialization failed");
        return -1;
    }

    if (sem_init(&response_empty, 0, QUEUE_SIZE) != 0) {
        perror("Response empty semaphore initialization failed");
        pthread_mutex_destroy(&response_mutex);
        return -1;
    }

    if (sem_init(&response_full, 0, 0) != 0) {
        perror("Response full semaphore initialization failed");
        sem_destroy(&response_empty);
        pthread_mutex_destroy(&response_mutex);
        return -1;
    }

    return 0;
}


/* Destroy response queue */
void response_queue_destroy(void)
{
    sem_destroy(&response_empty);
    sem_destroy(&response_full);
    pthread_mutex_destroy(&response_mutex);
}


/* Add response to response queue */
int enqueue_response(Response response)
{
    if (sem_wait(&response_empty) != 0) {
        perror("Response empty semaphore wait failed");
        return -1;
    }

    pthread_mutex_lock(&response_mutex);

    response_queue[response_rear] = response;
    response_rear = (response_rear + 1) % QUEUE_SIZE;

    pthread_mutex_unlock(&response_mutex);

    if (sem_post(&response_full) != 0) {
        perror("Response full semaphore post failed");
        return -1;
    }

    return 0;
}


/* Remove response from response queue */
int dequeue_response(Response *response)
{
    if (response == NULL) {
        return -1;
    }

    if (sem_wait(&response_full) != 0) {
        perror("Response full semaphore wait failed");
        return -1;
    }

    pthread_mutex_lock(&response_mutex);

    *response = response_queue[response_front];
    response_front = (response_front + 1) % QUEUE_SIZE;

    pthread_mutex_unlock(&response_mutex);

    if (sem_post(&response_empty) != 0) {
        perror("Response empty semaphore post failed");
        return -1;
    }

    return 0;
}
