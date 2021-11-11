# Shell Lab report

updating ... ...

## Problem Description

In this project, I will fulfill a simple Unix shell, where we would complete the empty function body list below:

• `eval`: Main routine that parses and interprets the command line. [70 lines]

• `builtin cmd`: Recognizes and interprets the built-in commands: quit, fg, bg, and jobs. [25 lines]

• `do bgfg`: Implements the bg and fg built-in commands. [50 lines]

• `waitfg`: Waits for a foreground job to complete. [20 lines]

• `sigchld handler`: Catches `SIGCHILD `signals. 80 lines]

• `sigint handler`: Catches `SIGINT `(ctrl-c) signals. [15 lines]

• `sigtstp handler`: Catches `SIGTSTP `(ctrl-z) signals. [15 lines]

All of these function is in `tsh.c`.

### The tsh Speciﬁcation

+ The prompt should be the string “tsh> ”.

+ The command line typed by the user should consist of a name and zero or more arguments, all separated by one or more spaces. If `name `is a **built-in command**, then `tsh `should handle it immediately and wait for the next command line. Otherwise, `tsh `should **assume that name is the path of an executable ﬁle**, which it loads and runs in the context of an initial child process (In this context, the term job refers to this initial child process).

+ Typing `ctrl-c (ctrl-z)` should cause a `SIGINT (SIGTSTP)` signal to be sent to the current foreground job, as well as any descendents of that job (e.g., any child processes that it forked). If there is no foreground job, then the signal should have no effect.

+ If the command line ends with an ampersand `&`, then tsh should run the job in the background.

  Otherwise, it should run the job in the foreground.

+ Each job can be identiﬁed by either a process ID (PID) or a job ID (JID), which is a positive integer assigned by tsh. JIDs should be denoted on the command line by the preﬁx ’%’. For example, “%5” denotes JID 5, and “5” denotes PID 5.

+ `tsh `should support the following built-in commands:
  + The `quit `command terminates the shell.
  + The `jobs `command lists all background jobs.
  + The bg `<job>` command restarts `<job>` by sending it a `SIGCONT `signal, and then runs it in the background. The `<job> `argument can be either a **PID** or a **JID**.
  + The fg `<job>` command restarts `<job>` by sending it a `SIGCONT `signal, and then runs it in the foreground. The `<job> `argument can be either a **PID** or a **JID**.
+ `tsh `should reap all of its zombie children. If any job terminates because it receives a signal that it didn’t catch, then tsh should recognize this event and print a message with the job’s PID and a description of the offending signal.

## Background

