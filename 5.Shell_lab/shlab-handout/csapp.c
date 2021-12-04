/* $begin csapp.c */
#include "csapp.h"

/************************** 
 * Error-handling functions
 **************************/
/* $begin errorfuns */
/* $begin unixerror */
void unix_error(char *msg) /* Unix-style error */
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}
/* $end unixerror */


void app_error(char *msg) /* Application error */
{
    fprintf(stderr, "%s\n", msg);
    exit(0);
}
/* $end errorfuns */



/*********************************************
 * Wrappers for Unix process control functions
 ********************************************/

/* $begin forkwrapper */
pid_t Fork(void) 
{
    pid_t pid;

    if ((pid = fork()) < 0)
	    unix_error("Fork error");
    return pid;
}
/* $end forkwrapper */

void Execve(const char *filename, char *const argv[], char *const envp[]) 
{
    if (execve(filename, argv, envp) < 0){
        printf("%s: Command not found\n", filename);
        exit(EXIT_FAILURE);
    }
}

/* $begin wait */
pid_t Wait(int *status) 
{
    pid_t pid;

    if ((pid  = wait(status)) < 0)
	    unix_error("Wait error");
    return pid;
}
/* $end wait */

pid_t Waitpid(pid_t pid, int *iptr, int options) 
{
    pid_t retpid;

    if ((retpid  = waitpid(pid, iptr, options)) < 0) 
	    unix_error("Waitpid error");
    return(retpid);
}

/* $begin kill */
void Kill(pid_t pid, int signum) 
{
    int rc;

    if ((rc = kill(pid, signum)) < 0)
	    unix_error("Kill error");
}
/* $end kill */

void Pause() 
{
    (void)pause();
    return;
}

unsigned int Sleep(unsigned int secs) 
{
    unsigned int rc;

    if ((rc = sleep(secs)) < 0)
	    unix_error("Sleep error");
    return rc;
}

unsigned int Alarm(unsigned int seconds) {
    return alarm(seconds);
}
 
void Setpgid(pid_t pid, pid_t pgid) {
    int rc;

    if ((rc = setpgid(pid, pgid)) < 0)
	    unix_error("Setpgid error");
    return;
}

pid_t Getpgrp(void) {
    return getpgrp();
}

/************************************
 * Wrappers for Unix signal functions 
 ***********************************/

void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
    if (sigprocmask(how, set, oldset) < 0)
	    unix_error("Sigprocmask error");
    return;
}

void Sigemptyset(sigset_t *set)
{
    if (sigemptyset(set) < 0)
	    unix_error("Sigemptyset error");
    return;
}

void Sigfillset(sigset_t *set)
{ 
    if (sigfillset(set) < 0)
	unix_error("Sigfillset error");
    return;
}

void Sigaddset(sigset_t *set, int signum)
{
    if (sigaddset(set, signum) < 0)
	    unix_error("Sigaddset error");
    return;
}

void Sigdelset(sigset_t *set, int signum)
{
    if (sigdelset(set, signum) < 0)
	    unix_error("Sigdelset error");
    return;
}

int Sigismember(const sigset_t *set, int signum)
{
    int rc;
    if ((rc = sigismember(set, signum)) < 0)
	    unix_error("Sigismember error");
    return rc;
}

int Sigsuspend(const sigset_t *set)
{
    int rc = sigsuspend(set); /* always returns -1 */
    if (errno != EINTR)
        unix_error("Sigsuspend error");
    return rc;
}

/*************************************************************
 * The Sio (Signal-safe I/O) package - simple reentrant output
 * functions that are safe for signal handlers.
 *************************************************************/

/* Private sio functions */

/* $begin sioprivate */
/* sio_reverse - Reverse a string (from K&R) */
static void sio_reverse(char s[])
{
    int c, i, j;

    for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/* sio_ltoa - Convert long to base b string (from K&R) */
static void sio_ltoa(long v, char s[], int b) 
{
    int c, i = 0;
    int neg = v < 0;

    if (neg)
	v = -v;

    do {  
        s[i++] = ((c = (v % b)) < 10)  ?  c + '0' : c - 10 + 'a';
    } while ((v /= b) > 0);

    if (neg)
	s[i++] = '-';

    s[i] = '\0';
    sio_reverse(s);
}

/* sio_strlen - Return length of string (from K&R) */
static size_t sio_strlen(char s[])
{
    int i = 0;

    while (s[i] != '\0')
        ++i;
    return i;
}
/* $end sioprivate */

/* Public Sio functions */
/* $begin siopublic */

ssize_t sio_puts(char s[]) /* Put string */
{
    return write(STDOUT_FILENO, s, sio_strlen(s)); //line:csapp:siostrlen
}

ssize_t sio_putl(long v) /* Put long */
{
    char s[128];
    
    sio_ltoa(v, s, 10); /* Based on K&R itoa() */  //line:csapp:sioltoa
    return sio_puts(s);
}

void sio_error(char s[]) /* Put error message and exit */
{
    sio_puts(s);
    _exit(1);                                      //line:csapp:sioexit
}
/* $end siopublic */

/*******************************
 * Wrappers for the SIO routines
 ******************************/
ssize_t Sio_putl(long v)
{
    ssize_t n;
  
    if ((n = sio_putl(v)) < 0)
	    sio_error("Sio_putl error");
    return n;
}

ssize_t Sio_puts(char s[])
{
    ssize_t n;
  
    if ((n = sio_puts(s)) < 0)
	    sio_error("Sio_puts error");
    return n;
}

void Sio_error(char s[])
{
    sio_error(s);
}

/********************************
 * Wrappers for Unix I/O routines
 ********************************/

int Dup2(int fd1, int fd2) 
{
    int rc;

    if ((rc = dup2(fd1, fd2)) < 0)
	    unix_error("Dup2 error");
    return rc;
}

/* $end csapp.c */




