#pragma once
#ifndef __SBUF__H
#define __SBUF__H

#include "csapp.h"

#define NTHREADS        4
#define SBUFSIZE        16
#define THREAD_LIMIT    4096

typedef struct
{
    int *buf;       /* Buffer array */
    int n;          /* Maxinum number of slots */
    int front;      /* buf[ ( front + 1 ) % n ] is the first item */
    int rear;       /* buf[ rear % n ] is the last item */
    sem_t mutex;    /* Protects accesses to buf */
    sem_t slots;    /* Counts available slots */
    sem_t items;    /* Counts available items */
} sbuf_t;

/* Sbuf function definition */
void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);
int is_sbuf_empty(sbuf_t *sp);
int is_sbuf_full(sbuf_t *sp);
void insert_wrapper(int fd);
void deinit_wrapper();

/* Thread function definition */
void *adjust_thread(void *vargp);
void *worker_thread(void* vargp);
void init_thread(void);
void create_worker_thread(int startPos, int endPos);
#endif /* __SBUF__H */