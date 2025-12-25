#include "segel.h"
#include "request.h"
#include "log.h"
#include "requestQueue.h"
#include <pthread.h>

//
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//


// Parses command-line arguments
void getargs(int *port, int *threadNum, int *qSize ,int argc ,char *argv[])
{
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *threadNum = atoi(argv[2]);
    *qSize = atoi(argv[3]);
}
// TODO: HW3 — Initialize thread pool and request queue
// This server currently handles all requests in the main thread.
// You must implement a thread pool (fixed number of worker threads)
// that process requests from a synchronized queue.
void* workerThreadFunction(void* arg){
    void** args = (void**) arg;
    struct RequestQueue* rq = (struct RequestQueue*) args[0];
    server_log log = (server_log)args[1];
    int threadId = *((int*)args[2]);

    threads_stats t = malloc(sizeof(struct Threads_stats));
    t->id = threadId;             // Thread ID (placeholder)
    t->stat_req = 0;       // Static request count
    t->dynm_req = 0;       // Dynamic request count
    t->total_req = 0;      // Total request count
    t->post_req = 0;       // POST request count


    //try to catch requests from the queue and handle them
    while(1) {
        int connfd = dequeue(rq);
        time_stats dum;
        requestHandle(connfd, dum, t, log);
        Close(connfd);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    // Create the global server log
    server_log log = create_log();

    int listenfd, connfd, port, clientlen, qSize, threadsNum;
    struct sockaddr_in clientaddr;
    //get the arguments
    getargs(&port, &threadsNum, &qSize, argc, argv);
    //initialize the request queue
    struct RequestQueue rq;
    queueInit(&rq, qSize);

    //intialize array of threads - the thread pool
    pthread_t* threads = malloc(sizeof(pthread_t) * threadsNum);
    for (int i = 0; i < threadsNum; ++i) {
        //create each worker thread arguments
        int* id_ptr = malloc(sizeof(int));
        *id_ptr = i;
        void** threadArgs = malloc(3 * sizeof(void*));
        threadArgs[0] = (void*)&rq;
        threadArgs[1] = (void*)log;
        threadArgs[2] = (void*)id_ptr;
        //create the worker thread
        if(pthread_create(&threads[i], NULL, workerThreadFunction, (void*)threadArgs) != 0) {
            fprintf(stderr, "Error creating thread\n");
            exit(1);
        }
    }




    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t*) &clientlen);
        enqueue(&rq, connfd);
        // TODO: HW3 — Record the request arrival time here

        // DEMO PURPOSE ONLY:
        // This is a dummy request handler that immediately processes
        // the request in the main thread without concurrency.
        // Replace this with logic to enqueue the connection and let
        // a worker thread process it from the queue.



        // gettimeofday(&arrival, NULL);


    }

    // Clean up the server log before exiting
    destroy_log(log);

    // TODO: HW3 — Add cleanup code for thread pool and queue
}
