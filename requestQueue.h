#ifndef OS3_REQUESTQUEUE_H
#define OS3_REQUESTQUEUE_H
#include <stdlib.h>
#include <pthread.h>

struct Node{
    int connfd;
    struct Node* next;
};

struct RequestQueue {
    //queue parameters
    int max_size;
    int current_size;
    struct Node* front;
    struct Node* back;
    int* buffer;
    //thread stuff
    pthread_mutex_t lock;
    pthread_cond_t notEmpty;
    pthread_cond_t notFull;
};

void queueInit(struct RequestQueue* q, int max_size);
void enqueue(struct RequestQueue* q, int item);
int dequeue(struct RequestQueue* q);
#endif //OS3_REQUESTQUEUE_H
