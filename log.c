#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "log.h"


//node struct for server log entries
typedef struct LogNode{
    char* entry;
    int len;
    struct LogNode* next;
} LogNode;

// Opaque struct definition
struct Server_Log {
    // TODO: Implement internal log storage (e.g., dynamic buffer, linked list, etc.) - done
    //linked list values
    LogNode* head;
    LogNode* tail;
    int total_chars;
    // thread locks and conditions
    pthread_mutex_t lock;
    pthread_cond_t can_read;
    pthread_cond_t can_write;
    // readers and writers stats
    int currentReaders;
    int waitingWriters;
    int currentWriters;
};

// Creates a new server log instance (stub)
server_log create_log() {
    // TODO: Allocate and initialize internal log structure - done
    server_log sl = (server_log)malloc(sizeof(struct Server_Log));
    sl->head = NULL;
    sl->tail = NULL;
    sl->total_chars = 0;
    sl->currentReaders = 0;
    sl->waitingWriters = 0;
    sl->currentWriters = 0;
    pthread_mutex_init(&sl->lock, NULL);
    pthread_cond_init(&sl->can_read, NULL);
    pthread_cond_init(&sl->can_write, NULL);
    return sl;
}

// Destroys and frees the log (stub)
void destroy_log(server_log log) {
    // TODO: Free all internal resources used by the log - done
    LogNode* tmp = log->head;
    while(tmp != NULL){
        LogNode* prev = tmp;
        tmp = tmp->next;
        free(prev->entry);
        free(prev);
    }
    free(log);
}

// Returns dummy log content as string (stub)
int get_log(server_log log, char** dst) {
    // TODO: Return the full contents of the log as a dynamically allocated string - done
    // This function should handle concurrent access
    //reader lock
    pthread_mutex_lock(&log->lock);
    while(log->currentWriters > 0 || log->waitingWriters > 0){
        pthread_cond_wait(&log->can_read,&log->lock);
    }
    log->currentReaders++;
    pthread_mutex_unlock(&log->lock);
    char* logData = malloc(log->total_chars + 1);
    char* currentLog = logData;
    LogNode* tmp = log->head;
    while(tmp != NULL){
        memcpy(currentLog,tmp->entry,tmp->len);
        currentLog += tmp->len;
        tmp = tmp->next;
    }
    logData[log->total_chars] = '\0';
    *dst = logData;
    int len = log->total_chars;
    //reader unlock
    pthread_mutex_lock(&log->lock);
    log->currentReaders--;
    if(log->currentReaders == 0){
        pthread_cond_signal(&log->can_write);
    }
    pthread_mutex_unlock(&log->lock);
    return len;
}

// Appends a new entry to the log (no-op stub)
void add_to_log(server_log log, const char* data, int data_len) {
    // TODO: Append the provided data to the log - done;
    // This function should handle concurrent access
    //writers lock
    pthread_mutex_lock(&log->lock);
    log->waitingWriters++;
    while(log->currentReaders > 0 || log->currentWriters > 0) {
        pthread_cond_wait(&log->can_write, &log->lock);
    }
    log->waitingWriters--;
    log->currentWriters = 1;
    pthread_mutex_unlock(&log->lock);
    //the new node in the log
    LogNode* newData = malloc(sizeof (struct LogNode));
    newData->entry = malloc(data_len + 1);
    newData->len = data_len;
    memcpy(newData->entry,data,data_len);
    newData->entry[data_len] = '\0';
    newData->next = NULL;
    //adding to the log
    if(log->tail == NULL){
        log->tail = newData;
        log->head = log->tail;
    }else{
        log->tail->next = newData;
        log->tail = newData;
    }
    log->total_chars += data_len;
    //get ready for the next request
    pthread_mutex_lock(&log->lock);
    log->currentWriters = 0;
    if(log->waitingWriters > 0){
        pthread_cond_signal(&log->can_write);
    }else{
        pthread_cond_broadcast(&log->can_read);
    }
    pthread_mutex_unlock(&log->lock);
}
