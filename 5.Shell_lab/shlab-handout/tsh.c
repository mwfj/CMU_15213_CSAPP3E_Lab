/*
 * tsh - A tiny shell program with job control
 *
 * @author Wufangjie Ma
 */

#include "csapp.h"


/* Misc manifest constants */
#define MAXLINE 1024   /* max line size */
#define MAXARGS 128    /* max args on a command line */
#define MAXJOBS 16     /* max jobs at any point in time */
#define MAXJID 1 << 16 /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/*
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;   /* defined in libc */
char prompt[] = "tsh> "; /* command line prompt (DO NOT CHANGE) */
int verbose = 0;         /* if true, print additional output */
int nextjid = 1;         /* next job ID to allocate */
char sbuf[MAXLINE];      /* for composing sprintf messages */
// Used for signal handler and blocking signal
volatile sig_atomic_t flag;

struct job_t
{                          /* The job struct */
    pid_t pid;             /* job PID */
    int jid;               /* job ID [1, 2, ...] */
    int state;             /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE]; /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */

/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv);
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs);
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid);
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid);
int pid2jid(pid_t pid);
void listjobs(struct job_t *jobs);

void usage(void);

typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine
 */
int main(int argc, char **argv)
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    // dup2(1, 2);
    Dup2(1,2);
    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF)
    {
        switch (c)
        {
        case 'h': /* print help message */
            usage();
            break;
        case 'v': /* emit additional diagnostic info */
            verbose = 1;
            break;
        case 'p':            /* don't print a prompt */
            emit_prompt = 0; /* handy for automatic testing */
            break;
        default:
            usage();
        }
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT, sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler); /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler); /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler);

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1)
    {
        /* Read command line */
        if (emit_prompt)
        {
            printf("%s", prompt);
            fflush(stdout);
        }
        if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
            app_error("fgets error");
        if (feof(stdin))
        { /* End of file (ctrl-d) */
            fflush(stdout);
            exit(0);
        }

        /* Evaluate the command line */
        eval(cmdline);
        fflush(stdout);
        fflush(stdout);
    }

    exit(0); /* control never reaches here */
}

/*
 * eval - Evaluate the command line that the user has just typed in
 *
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.
 */
void eval(char *cmdline)
{
    // To get the separatable command line
    char *argv[MAXARGS];
    int bg = parseline(cmdline, argv);
    if(argv[0] == NULL)
        return ;

    pid_t pid;

    // Running the custom command
    // Prevent signal race
    sigset_t  mask_all, mask, prev_all;
    // Add every signal number to set
    Sigfillset(&mask_all);
    // Init mask
    Sigemptyset(&mask);
    // Set the mask
    Sigaddset(&mask, SIGCHLD);
    Sigemptyset(&prev_all);

    if(!builtin_cmd(argv)){
        // Block the Signal to make sure all the signal received
        Sigprocmask(SIG_BLOCK, &mask, NULL);
        // Create a child process to do the job
        if( (pid = Fork() ) == 0){
            // The child process inherit the signal from the parent
            // unblock this signal
            Sigprocmask(SIG_SETMASK,&prev_all, NULL);
            // Set the process group of the current process
            // To make sure each process in the independent group
            // and have a unique groupID
            Setpgid(0, 0);
            // Replace the address space with the new program
            Execve(argv[0], argv, environ);
        }else{
            // entere the parent process
            // Add the new job
            // Note the need to block all signal when you add the job
            // Sigprocmask(SIG_BLOCK, &mask_all, NULL);
            if(bg)
                // Background Job
                addjob(jobs, pid, BG, cmdline);
            else{
                flag = 0;
                // Foregrond Job
                // The shell must wait the child process until it finished.
                addjob(jobs, pid, FG, cmdline);
            }
            Sigprocmask(SIG_UNBLOCK, &prev_all, NULL);

            if(bg){
                int jid = pid2jid(pid);
                printf("[%d] (%d) %s", jid, pid, cmdline);
            }else
                // Notice that the signal will not be blocked until the child process finished
                // Wait until the child process finished
                waitfg(pid);
            
        }

    }
    return;
}

/*
 * parseline - Parse the command line and build the argv array.
 *
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.
 */
int parseline(const char *cmdline, char **argv)
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf) - 1] = ' ';   /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'')
    {
        buf++;
        delim = strchr(buf, '\'');
    }
    else
    {
        delim = strchr(buf, ' ');
    }

    while (delim)
    {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* ignore spaces */
            buf++;

        if (*buf == '\'')
        {
            buf++;
            delim = strchr(buf, '\'');
        }
        else
        {
            delim = strchr(buf, ' ');
        }
    }
    argv[argc] = NULL;

    if (argc == 0) /* ignore blank line */
        return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc - 1] == '&')) != 0)
    {
        argv[--argc] = NULL;
    }
    return bg;
}

/*
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.
 */
int builtin_cmd(char **argv)
{
    // Quit the command 
    if(!strcmp(argv[0],"quit") || !strcmp("exit", argv[0])){
        if(verbose){
            printf("quit the shell.\n");
        }
        exit(EXIT_SUCCESS);
    }else if(!strcmp("&",argv[0])){
        if(verbose){
            printf("Run a process in the background.\n");
        }
        return 1;
    }else if(!strcmp("fg",argv[0]) || !strcmp("bg", argv[0])){
        if(verbose){
            printf("Run a process in the foreground.\n");
        }
        do_bgfg(argv); 
        return 1;
    }else if(!strcmp("jobs",argv[0])){
        if(verbose){
            printf("List all the current jobs:\n");
        }
        listjobs(jobs);
        return 1;
    }else{
        return 0; /* not a builtin command */
    }
}

/*
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv)
{
    // In this function, we assumed that the argv[0]
    // should be "bg/fg"
    // and thus we start get the command begin with argv[1]
    char* command = argv[1];
    // Init pid
    pid_t cur_pid = -1;

    struct job_t* cur_job;
    if(command == NULL){
        printf("%s command requires PID or %%jobid argument\n", argv[0]);
        return ;
    }
    
    // The case of the command is jid
    if(command[0] == '%'){
        // Change the job id from string to integer
        // start, which skip to '%'
        int cur_jid = atoi(&command[1]);
        // Get the job
        cur_job = getjobjid(jobs, cur_jid);
        // Make sure the job is exist
        if(cur_job == NULL){
            printf("%s no such job\n", command);
            return ;
        }
        cur_pid = cur_job -> pid;

    }else if(isdigit(command[0])){
        // The case of the command is pid
        int cur_pid = atoi(command);
        cur_job = getjobpid(jobs, cur_pid);
        // Make sure the job is exist
        if(cur_job == NULL){
            printf("%s no such job\n", command);
            return ;
        }
    }else{
        // Handle the Error condition
        printf("%s: the argument should be jid or pid\n",argv[0]);
        return ;
    }
    // Send the signal to revoke the current child process
    kill(-cur_pid, SIGCONT);

    // Background job
    if(!strcmp(argv[0], "bg")){
        printf("[%d] (%d) %s",cur_job ->jid, cur_job->pid, cur_job->cmdline);
        cur_job -> state =  BG;
    }else{
        // Foreground job
        printf("[%d] (%d) %s",cur_job ->jid, cur_job->pid, cur_job->cmdline);
        flag = 0;
        cur_job -> state = FG;
        // Wait the until the job finish
        waitfg(cur_pid);
    }

    return;
}

/*
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid)
{
    if(!pid)
        return ;
    struct job_t* cur_job = getjobpid(jobs, pid);
    sigset_t mask;
    Sigemptyset(&mask);
    // Make sure the current pid has the validate job
    if(cur_job != NULL){
        // If the current pid is the fg pid 
        // then wait it until it suspend or stop
        // while(pid == fgpid(jobs)){
        //     Sigsuspend(&mask);
        // }
        while(!flag){
            Sigsuspend(&mask);
        }
    }
    return ;
}

/*****************
 * Signal handlers
 *****************/

/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.
 */
void sigchld_handler(int sig)
{
    int olderrno = errno;

    pid_t pid;
    int status;

    // Detect whether the child stop or terminate
    while((pid = waitpid(-1,&status, WNOHANG | WUNTRACED)) > 0){
        
        if(pid == fgpid(jobs)){
            flag = 1;
        }

        sigset_t mask_all, prev_all;
        // Sigemptyset(&mask_all);
        Sigfillset(&mask_all);
        
        struct job_t* cur_job = getjobpid(jobs, pid);

        if(cur_job == NULL){
            Sio_error("ERROR: no such job with the specified PID.\n");
        }else{
            // The child process return normally
            if(WIFEXITED(status)){
                Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
                deletejob(jobs, pid);
                Sigprocmask(SIG_SETMASK, &prev_all, NULL);
            }
            // The child process blocked by signal
            else if(WIFSIGNALED(status)){
                int jid = pid2jid(pid);
                printf("Job [%d] (%d) terminated by signal %d\n",jid,pid,WTERMSIG(status));
                Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
                deletejob(jobs, pid);
                Sigprocmask(SIG_SETMASK, &prev_all, NULL);
            }
            // The child process terminated
            else if(WIFSTOPPED(status)){
                Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
                cur_job -> state = ST;
                Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
                int jid = pid2jid(pid);
                printf("Job [%d] (%d) terminated by signal %d\n",jid,pid,WSTOPSIG(status));
            }
        }
    }

    errno = olderrno;

    return;
}

/*
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.
 */
void sigint_handler(int sig)
{
    int olderrno = errno;
    pid_t pid = fgpid(jobs);
    if(pid <= 0)
        return ;
    Kill(pid, SIGINT);
    errno = olderrno;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.
 */
void sigtstp_handler(int sig)
{
    int olderrno = errno;
    pid_t pid = fgpid(jobs);
    if(pid <=0)
        return ;
    Kill(pid, SIGTSTP);
    errno = olderrno;
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job)
{
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs)
{
    int i;

    for (i = 0; i < MAXJOBS; i++)
        clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs)
{
    int i, max = 0;

    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].jid > max)
            max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline)
{
    int i;

    if (pid < 1)
        return 0;

    for (i = 0; i < MAXJOBS; i++)
    {
        if (jobs[i].pid == 0)
        {
            jobs[i].pid = pid;
            jobs[i].state = state;
            jobs[i].jid = nextjid++;
            if (nextjid > MAXJOBS)
                nextjid = 1;
            strcpy(jobs[i].cmdline, cmdline);
            if (verbose)
            {
                printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
        }
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid)
{
    int i;

    if (pid < 1)
        return 0;

    for (i = 0; i < MAXJOBS; i++)
    {
        if (jobs[i].pid == pid)
        {
            clearjob(&jobs[i]);
            nextjid = maxjid(jobs) + 1;
            return 1;
        }
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs)
{
    int i;

    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].state == FG)
            return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid)
{
    int i;

    if (pid < 1)
        return NULL;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].pid == pid)
            return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid)
{
    int i;

    if (jid < 1)
        return NULL;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].jid == jid)
            return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid)
{
    int i;

    if (pid < 1)
        return 0;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].pid == pid)
        {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs)
{
    int i;

    for (i = 0; i < MAXJOBS; i++)
    {
        if (jobs[i].pid != 0)
        {
            printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
            switch (jobs[i].state)
            {
            case BG:
                printf("Running ");
                break;
            case FG:
                printf("Foreground ");
                break;
            case ST:
                printf("Stopped ");
                break;
            default:
                printf("listjobs: Internal error: job[%d].state=%d ",
                       i, jobs[i].state);
            }
            printf("%s", jobs[i].cmdline);
        }
    }
}
/******************************
 * end job list helper routines
 ******************************/

/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void)
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
/* $begin sigaction */

handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* Block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* Restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}
/* $end sigaction */
/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig)
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}