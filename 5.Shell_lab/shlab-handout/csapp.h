/*
 * csapp.h - prototypes and definitions for the CS:APP3e book
 */
/* $begin csapp.h */
#ifndef __CSAPP_H__
#define __CSAPP_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h> // sigset type

void unix_error(char *msg);
void app_error(char *msg);


void Sigprocmask(int, const sigset_t *, sigset_t *);
void Sigemptyset(sigset_t*);
void Sigfillset(sigset_t *);
void Sigaddset(sigset_t*, int);
int Sigsuspend(const sigset_t*);

/* Unix control function wrapper */
pid_t Fork(void);
void Execve(const char *, char *const[], char *const[]);
void Kill(pid_t, int);
void Setpgid(pid_t, pid_t);

ssize_t sio_puts(char[]);
void sio_error(char[]);
void Sio_error(char[]);

int Dup2(int, int);

#endif /* __CSAPP_H__ */
/* $end csapp.h */
