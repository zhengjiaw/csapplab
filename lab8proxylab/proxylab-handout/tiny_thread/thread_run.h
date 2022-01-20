#ifndef THREAD_RUN_H
#define THREAD_RUN_H
#include <stdio.h>

#include "csapp.h"
#include "sbuf.h"

#define SBUFSIZE 4
#define INIT_THREAD_N 1
#define THREAD_LIMIT 4096

extern sbuf_t sbuf; /* Shared buffer of connected descriptors */

// thread info
typedef struct {
    pthread_t tid;
} ithread;

// init work
void init(void);
// function for create server thread
void *serve_thread(void );
/*
 * creating thread that adjust total thread count according to sbuf situation
 *
 * if sbuf is empty, double threads
 * if sbuf is full, half threads
 */
void *adjust_threads(void *);
// from start to end, create (end - start) new server threads
void create_threads(int start, int end);

#endif