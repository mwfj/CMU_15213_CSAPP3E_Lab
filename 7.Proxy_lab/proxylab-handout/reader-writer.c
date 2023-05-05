#include "reader-writer.h"

void *
init_wr_locks(){
    Sem_init(&mutex, 0, 1);
    Sem_init(&w, 0, 1);
    is_prev_write = 0;
    readcnt = 0;
    return (void*)NULL;
}

inline void 
lock_reader(void){
    P(&mutex);
    ++readcnt;
    if(readcnt == 1)    P(&w);
    V(&mutex);
    P(&mutex);
}

inline void 
unlock_reader(void){
    if(--readcnt == 0)  V(&w);
    is_prev_write = 0;
    V(&mutex);
}

inline void 
lock_writer(void){
    while(is_prev_write && readcnt)
        ;
    P(&w);
}

inline void 
unlock_writer(void){
    is_prev_write = 1;
    V(&w);
}