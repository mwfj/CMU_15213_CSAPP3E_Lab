#ifndef __READER_WRITER_H__
#define __READER_WRITER_H__

#include <semaphore.h>
#include "csapp.h"

sem_t mutex, w;
unsigned volatile int readcnt;
unsigned volatile char is_prev_write;

void lock_reader(void);
void unlock_reader(void);

void lock_writer(void);
void unlock_writer(void);

void *init_wr_locks();

#endif /* __READER_WRITER_H__ */