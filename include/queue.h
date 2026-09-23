#ifndef QUEUE_H
#define QUEUE_H

#include "common.h"

#define QUEUE_SIZE 50

/* Request queue functions */
int request_queue_init(void);
void request_queue_destroy(void);

int enqueue_request(Request request);
int dequeue_request(Request *request);

/* Response queue functions */
int response_queue_init(void);
void response_queue_destroy(void);

int enqueue_response(Response response);
int dequeue_response(Response *response);

#endif
