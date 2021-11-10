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

## Background

