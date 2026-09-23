#include <stdio.h>
#include <pthread.h>

#include "queue.h"

void *producer(void *arg)
{
    (void)arg;

    for (int i = 1; i <= 5; i++) {
        Request request;

        request.client_fd = i;

        snprintf(
            request.request,
            sizeof(request.request),
            "Test request %d",
            i
        );

        enqueue_request(request);

        printf("[Producer] Added: %s\n", request.request);
    }

    return NULL;
}


void *consumer(void *arg)
{
    (void)arg;

    for (int i = 1; i <= 5; i++) {
        Request request;

        dequeue_request(&request);

        printf(
            "[Consumer] Received from client %d: %s\n",
            request.client_fd,
            request.request
        );
    }

    return NULL;
}


int main(void)
{
    pthread_t producer_thread;
    pthread_t consumer_thread;

    printf("Initializing request queue...\n");

    if (request_queue_init() != 0) {
        return 1;
    }

    printf("Queue initialized successfully.\n\n");

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    request_queue_destroy();

    printf("\nRequest queue test completed successfully.\n");

    return 0;
}
