#include "requestQueue.h"


void queueInit(struct RequestQueue* q, int max_size){
    q->max_size = max_size;
    q->current_size = 0;
    q->front = NULL;
    q->back = NULL;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->notEmpty, NULL);
    pthread_cond_init(&q->notFull, NULL);
}

void enqueue(struct RequestQueue* q, int item,struct timeval arrivalTime){
    pthread_mutex_lock(&q->lock);
    while(q->current_size == q->max_size){
        pthread_cond_wait(&q->notFull, &q->lock);
    }
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->connfd = item;
    newNode->next = NULL;
    newNode->arrivalTime = arrivalTime;
    if(q->current_size == 0){
        q->front = newNode;
        q->back = newNode;
    } else {
        q->back->next = newNode;
        q->back = newNode;
    }
    q->current_size++;
    pthread_cond_signal(&q->notEmpty);
    pthread_mutex_unlock(&q->lock);
}


int dequeue(struct RequestQueue* q,struct timeval* arrivalTime){
    pthread_mutex_lock(&q->lock);
    while(q->current_size == 0){
        pthread_cond_wait(&q->notEmpty, &q->lock);
    }
    struct Node* tmp = q->front;
    int item = tmp->connfd;
    if(arrivalTime != NULL){
        *arrivalTime = tmp->arrivalTime;
    }
    q->front = q->front->next;
    free(tmp);
    q->current_size--;
    if(q->current_size == 0){
        q->back = NULL;
    }
    pthread_cond_signal(&q->notFull);
    pthread_mutex_unlock(&q->lock);
    return item;
}
