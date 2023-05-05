#include "sbuf.h"

extern void process_request(int fd);

static sbuf_t sbuf; /* shared buff between different connected descriptors */
static pthread_t thread_pool_ids[THREAD_LIMIT];
static int current_nthreads; /* Track the current thread number */


/**
 ****************************************************************************
 * 
 * Sbuf function implementation
 * 
 ****************************************************************************
 */

/* Create an empty, bounded, shared FIFO buffer with n slots */
void 
sbuf_init(sbuf_t *sp, int n){
    sp->buf = Calloc(n, sizeof(int));
    sp->n = n;                  /* Buffer holds max of n items */
    sp->front = sp->rear = 0;   /* Empty buffer when front == rear */
    sem_init(&sp->mutex, 0, 1); /* Binary semaphore for locking */
    sem_init(&sp->slots, 0, n); /* Initially, buf has n empty slots */
    sem_init(&sp->items, 0, 0); /* Initially, buf has zero data items */
}

/* Clean up buffer sp */
void 
sbuf_deinit(sbuf_t *sp){
    Free(sp->buf);
}

/* Insert the item onto the rear of shared buffer sp */
void
sbuf_insert(sbuf_t *sp, int item){
    P(&sp->slots);                          /* Wait for available slot */
    P(&sp->mutex);                          /* Lock the buffer */
    sp->buf[(++sp->rear) % (sp->n)] = item; /* Insert the data item */
    V(&sp->mutex);                          /* Unlock the buffer */
    V(&sp->items);                          /* Announce available slot */
}

/* Remove the item from the front of the shared buffer sp */
int
sbuf_remove(sbuf_t *sp){
    int item;
    P(&sp->items);                            /* Wait for available slot */
    /* the thread cannot be canceled any time when removing the sbuf */
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    P(&sp->mutex);                            /* Lock the buffer */
    item = sp->buf[(++sp->front) % (sp->n)];  /* Remove the data item */
    V(&sp->mutex);                            /* Unlock the buffer */
    V(&sp->slots);                            /* Announce available slot */
    return item;
}

/* Check the current sbuf capacity is empty or not */
int is_sbuf_empty(sbuf_t *sp){
    int is_empty;
    P(&sp ->mutex);                      /* Lock the buffer */
    sem_getvalue(&sp->items, &is_empty); /* Get the number of current used data item */
    V(&sp ->mutex);                      /* Unlock the buffer */
    return (is_empty == 0);
}

/* Check the current sbuf capacity is full or not */
int is_sbuf_full(sbuf_t *sp){
    int is_full;
    P(&sp->mutex);                      /* Lock the buffer */
    sem_getvalue(&sp->slots, &is_full); /* Get the number of  current slot item */
    V(&sp->mutex);                      /* Unlock the buffer */
    return (is_full == 0);
}

/* wrapper for sbuf_insert, call by outside */
void 
insert_wrapper(int fd){
    sbuf_insert(&sbuf, fd);
}

/* wrapper for sbuf_deinit, call by outside */
void 
deinit_wrapper(){
    sbuf_deinit(&sbuf);
}

/**
 ****************************************************************************
 * 
 * Thread function implementation
 * 
 ****************************************************************************
 */

void create_worker_thread(int startPos, int endPos){
    int i;
    for(i = startPos; i < endPos; i++){
        /* Create worker thread */
        Pthread_create(&thread_pool_ids[i], NULL, worker_thread, NULL);
    }
}

void init_thread(void){
    /* Init sbuf block */
    sbuf_init(&sbuf, SBUFSIZE);
    current_nthreads = NTHREADS;
    /* Init worker threads */
    create_worker_thread(0, current_nthreads);
}

void*
adjust_thread(void* vargp){
    sbuf_t *sp = &sbuf;
    while(1){
        /* If the current sbuf is full, double the thread worker number */
        if(is_sbuf_full(sp)){
            if(current_nthreads == THREAD_LIMIT){
                fprintf(stderr, "too many workers, cannot expand the thread number");
                continue;
            }
            /* Double thread number */
            int new_nthread = 2 * current_nthreads;
            create_worker_thread(current_nthreads, new_nthread);
            current_nthreads = new_nthread;
        }
        /* Cut the thread number by half */
        else if(is_sbuf_empty(sp)){
            /* If the current thread reach the minimum worker number, the do nothing */
            if(current_nthreads == NTHREADS) continue;
            /* Otherwise, cut hale thread workers */
            int new_nthread = (current_nthreads / 2);
            /**
             * keep [0, new_nthread];
             * kill [new_nthread, current_threads];
             */
            int i;
            for(i=new_nthread; i<current_nthreads; i++){
                Pthread_cancel(thread_pool_ids[i]);
            }
            current_nthreads = new_nthread;
        }
        /* Otherwise, keep the current thread number */
    }
}

void*
worker_thread(void* vargp){
    /* Independent the current worker thread */
    Pthread_detach(Pthread_self());
    while(1){
        int connfd = sbuf_remove(&sbuf);
        process_request(connfd);
        Close(connfd);
        /* worker thread can be cancel any time */
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    }

}