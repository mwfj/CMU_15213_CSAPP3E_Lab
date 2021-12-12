## Table of Content

<a name="index">**Index**</a>

<a href="#0">Shell Lab report</a>  
&emsp;<a href="#1">Problem Description</a>  
&emsp;&emsp;<a href="#2">The tsh Speciﬁcation</a>  
&emsp;<a href="#3">Background</a>  
&emsp;&emsp;<a href="#4">Exceptional Control Flow</a>  
&emsp;&emsp;<a href="#5">Exceptions : Hardware and operating system kernel software</a>  
&emsp;&emsp;<a href="#6">Exception Handler</a>  
&emsp;&emsp;&emsp;<a href="#7">Difference between **Exception** and **Procedure Call**:</a>  
&emsp;&emsp;<a href="#8">Interrupt(Asynchronous Exceptions)</a>  
&emsp;&emsp;<a href="#9">Synchronously Exceptions(Faulting Instruction)</a>  
&emsp;&emsp;&emsp;<a href="#10">Traps and System Calls</a>  
&emsp;&emsp;&emsp;<a href="#11">Faluts</a>  
&emsp;&emsp;&emsp;<a href="#12">Aborts</a>  
&emsp;&emsp;<a href="#13">Process</a>  
&emsp;&emsp;&emsp;<a href="#14">User and Kernel Mode</a>  
&emsp;&emsp;<a href="#15">Context Switch : Hardware timer and kernel software</a>  
&emsp;&emsp;<a href="#16">Process Control</a>  
&emsp;&emsp;&emsp;<a href="#17"> `exit()`</a>  
&emsp;&emsp;&emsp;<a href="#18">`fork()`: </a>  
&emsp;&emsp;&emsp;<a href="#19">`wait()`</a>  
&emsp;&emsp;&emsp;<a href="#20">`execve()`</a>  
&emsp;&emsp;<a href="#21">Shell</a>  
&emsp;&emsp;<a href="#22">Signal : kernel software and application software</a>  
&emsp;&emsp;&emsp;<a href="#23">Sending Signal</a>  
&emsp;&emsp;&emsp;&emsp;<a href="#24">Process Group</a>  
&emsp;&emsp;&emsp;<a href="#25">Receiving a Signal</a>  
&emsp;&emsp;&emsp;<a href="#26">Pending and Blocked Signal</a>  
&emsp;&emsp;&emsp;<a href="#27">Guidelines for Wri&ng Safe Handlers</a>  
&emsp;<a href="#28">Solution</a>  
&emsp;&emsp;<a href="#29">Check the built-in function</a>  
&emsp;&emsp;<a href="#30">SIGINT handler:</a>  
&emsp;&emsp;<a href="#31">SIGTSTP handler: </a>  
&emsp;&emsp;<a href="#32">SIGCHLD handler</a>  
&emsp;&emsp;<a href="#33">Waitfg</a>  
&emsp;&emsp;<a href="#34">do_fgbg</a>  
&emsp;&emsp;<a href="#35">Eval:</a>  
&emsp;<a href="#36">Test Our Shell</a>  

# <a name="0">Shell Lab report</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

<a name="1">Problem Description</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

In this project, I will fulfill a simple Unix shell, where we would complete the empty function body list below:

• `eval`: Main routine that parses and interprets the command line. [70 lines]

• `builtin cmd`: Recognizes and interprets the built-in commands: quit, fg, bg, and jobs. [25 lines]

• `do bgfg`: Implements the bg and fg built-in commands. [50 lines]

• `waitfg`: Waits for a foreground job to complete. [20 lines]

• `sigchld handler`: Catches `SIGCHILD `signals. 80 lines]

• `sigint handler`: Catches `SIGINT `(ctrl-c) signals. [15 lines]

• `sigtstp handler`: Catches `SIGTSTP `(ctrl-z) signals. [15 lines]

All of these function is in `tsh.c`.

### <a name="2">The tsh Speciﬁcation</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

+ The prompt should be the string `“tsh> ”`.

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

## <a name="3">Background</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

In this section, I just briefly summary some main conception for the exception and the signal. If you would like see more detail, please see the reference book: [CS:APP3e](http://csapp.cs.cmu.edu/3e/home.html).

### <a name="4">Exceptional Control Flow</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

From startup to shutdown, a CPU simply reads and executes(interprets) a sequence of instructions `i`, and one time, where it change the address `a_k` to `a_(k+1)`. Those instructions call the **control flow**. 

Up to now, there have two mechanisms for **changing the control flow**:

+ Jumps and branches
+ Call and return

React to changes in **program state**. Normally, the instruction `ik` and `ik+1` are adjacent with each other in the memory address and do the smooth flow changes unless an abrupt change occurred in the control flow.

However, only these control flows are not enough, because it is hard to deal with the **system state changes**, which is not related to the execution of the program. Such as I/O operation or data comes in the network adapter. This makes us introduce **Exceptional Control Flow(ECF)**. Exceptional Control flow occurs at all levels of a computer system.

+ At the **hardware level**, event detected by the **hardware trigger** abrupt control transfers to the exceptional handler.
+ At the **operating system level**, the kernel transfer control from one user process to another via **context switch**.
+ At the **application level**, a prcess can send a signal to another process that abruptly transfer control  to a **signal handler** in the recipient.
+ An **individual program** can react to errors by **sidestepping the usual call/return stack discipline** and **making nonlocal jumps** to arbitrary locations in other functions.

Applications request service from the operating systems(OS kernel) by using a form of ECF known as a **trap** or **system call**.

### <a name="5">Exceptions : Hardware and operating system kernel software</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

The exception implemented partly by the hardware and the operating system. Also, an exception is a transfer of control to OS kernel in response to some change in the processor's state, where the state is encoded in various significant bits and siginal inside the process. Note that the change in state is known as an event. The event might directly related to the execution of the current instrcution.

When the processor detects that the event has occurred, it makes an indirect procedure call, through the jump table called the exception table, to operating system subrountine(exception handler) that is specifically designed to process this particular kind of event.

When the exception occurred, one of three things happened, which depending on the type of event that caused exception:

1. The handler returns control to the current instrcution `I_curr`, the instruction that was executing when the event not occurred.
2. The handler returns contorl to `I_next`, the instruction that would have executed next had the exception not occurred.
3. The handler aborts the interrupted program.

![exception_state_transfer](/Users/mwfj/Desktop/readme-pic/exception_state_change.png)

<p align="center">The exception state transfer, the figure from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/14-ecf-procs.pdf">cmu-213 slide</a></p>

Exception can be divided into four classes:

1. interrupts
2. traps
3. faluts
4. aborts

### <a name="6">Exception Handler</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

Each type of possible exception in a system is assigned a **unique nonnegative integer exception number**. Some of these numbers are assigned by the designers of the **process**, whereas other numbers are assigned by the **operating system** kernel(the memory-resident part of the operating system). 

At **system boot time**, the operating system allocates and initializes a **jump table** called **exception table** so that entry k contains the address of the handler of exception k. At **run time**, the processor detects that an event has occurred and determines the corresponding exception number k. The processor then triggers the exception by making an indirect procedual call, through entry k of the exception table, to the exception handler. Specifially, **the exception number is the index of exception table**, whose starting address is contained in a special CPU resgister called ***exception table base register***.

Once the hardware triggers the exception, the exception handler start to process the event. After that, it optionally returns to the interrupted program by executing a special **"return from interrupt"** instruction, which pops the appropriate state back into the process control and data register, restore the state to the user mode if the exception interrupted a user program, and then returns control to the interrupted program.

<p align="center"> 
  <img src="./readme-pic/exception_table.png" alt="exception_work_flow" />
</p>


<p align="center">The exception table, the figure from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/14-ecf-procs.pdf">cmu-213 slide</a></p>

<p align="center"> <img src="./readme-pic/exception_work_flow.png" alt="exception_work_flow" style="zoom:80%;"/> </p>

#### <a name="7">Difference between **Exception** and **Procedure Call**:</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

+ **Return Address**: as with the **procedural call**, the processor pushes a return address on the stack before branching to the handler. Whereas, for the exception, the return address id either the current instruction or the next instruction.
+ The processor also pushes some additional processor state onto the stack that will be necessary to restart the interrupted program when the handler returns.
+ When the control is being transferred from a user program to the kernel, all of these items are pushed onto **the kernel's stack rather than user's stack.**
+ Exception handler run in **kernel mode**. 

**The exception can divided by:**

1. Asynchronous Exceptions:
   + Interrupts
2. Synchronous Exceptions:
   + Traps
   + Faults
   + Aborts

### <a name="8">Interrupt(Asynchronous Exceptions)</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

Interrupts occur asynchronously as a result of signal from I/O devices that are external to the processor.

Process:

1. Hardware Interrupt are asychronous in the sense that they are not caused by the execution of any particular instruction.Exception handlers for hardare interrupt are often called **interrupt handlers**.
2. The processor notices that the **interrupt pin has go high**, reads the exception number from system bus.
3. The processor calls the appropriate  interrupt handler.
4. When handler returns, it returns the control to the next instruction in the control flow.

<p align="center"> 
  <img src="./readme-pic/interrupt_work_flow.JPG" alt="interrupt_work_flow" />
</p>


<p align="center">The interrupt work flow, this figure is from the book <a href = "http://csapp.cs.cmu.edu/3e/home.html">CS:APP3e</a>  chapter 8</p>

### <a name="9">Synchronously Exceptions(Faulting Instruction)</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

#### <a name="10">Traps and System Calls</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

Traps are intentional exceptions that occur as a result of executing an instruction. Like interrupt handlers, trap handlers return contorl to the next instruction. **The most important use of traps is to provide a procedural-like interface between user program and the kernel**, known as **system call**.

To allow controlled access to such kernel services, process provides a special `syscall` instruction that user programs can execute when they are request the related service. Executing the `syscall` instruction cause a trap to an exception handler taht decodes the argument and calls the appropriate kernel routine.

From the programmer's perspective, a system call is identical to a regular function call, where the regular function runs in the **user mode**, which restricts the types of some higher privilege instructions they can execute, and they access the same stack as the calling function. Whereas the system call runs in the **kernel mode**, which allows it executes privileged instrctions and access a stack defined in the kernel.

By convention, register `%rax` contains the syscall number, with up to six arguements: `%rdi`, `%rsi`,`%rdx`, `%r10`,`%r8` and `%r9`. On return from the system call, register `%rcx` and `%r11` are destroyed, and `%rax` contains the return value. Negative value is an error corresponding to negative `errno`.

<p align="center"> 
  <img src="./readme-pic/traps_work_flow.JPG" alt="traps_work_flow" />
</p>


<p align="center">The traps work flow, this figure is from the book <a href = "http://csapp.cs.cmu.edu/3e/home.html">CS:APP3e</a>  chapter 8</p>

#### <a name="11">Faluts</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

Faults result from **error conditions** that a handler might be able to correct. When fault occurred, the processor transfers control to the fault handler. If the fault handler is able to correct the error condition, and then it returns control to the faulting instruction. Otherwise, the handler returns to an `abort` routine in the kernel that terminates the application program that caused fault.

<p align="center"> 
  <img src="./readme-pic/fault_work_flow.JPG" alt="fault_work_flow" />
</p>


<p align="center">The fault work flow, this figure is from the book <a href = "http://csapp.cs.cmu.edu/3e/home.html">CS:APP3e</a>  chapter 8</p>

#### <a name="12">Aborts</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

Aborts result from unrecoverable fatal errors, typically hardware errors. **Aborts handlers never return control to the application program**.

<p align="center"> 
  <img src="./readme-pic/abort_work_flow.JPG" alt="abort_work_flow" />
</p>


<p align="center">The fault work flow, this figure is from the book <a href = "http://csapp.cs.cmu.edu/3e/home.html">CS:APP3e</a>  chapter 8</p>

### <a name="13">Process</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

A process is an **instance of a program in execution**. Each program in the system runs in the **context** of some process. The context consist of the state that the program needs to run correctly, where the state includes:

+ the program's code 
+ data stored in memory
+ its stack
+ the contents of the general-purpose register
+ its program counter
+ environment variables
+ the set of open file descriptors.

Each time a user runs a program by typing the name of an executable object file to the shell, and then shell creates a new process and then runs the executable object file in the context of this new process. Application programs can also create new processes and run either their own code or other applications in the context of the new process.

Process provides each program with **two key abstractions**:

+ **Logical control flow:**

  The logical control flow is a **sequence of program counter values** that corresponded exclusively to instructions contained in our program's executable object file or in shared linked into our program dynamically at run time, where program counter is a 16 bit register that holds the address of the next instruction.

+ **Private address space:**

  Each program seems to have exclusive use of main memory. This space is private in the sense that a byte of memory associated with a particular address in the space cannot in general be read or write by any other process. The private address space is provided by kernel mechanism called virtual memory.

#### <a name="14">User and Kernel Mode</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

The processor typically provides the capability of restricting the instruction that an application can execute, where this capability set by a mode bit in some control register characterizes the privileges. Specifically, when the bit is set, the process is running in `kernel mode`, otherwise, the process running in `user mode`. A process running in the `user mode` is not allowed to execute privileged instructions nor is it allowed to directly reference code or date in the kernel area of the address space, whereas the process in the `kernel mode` can execute any instruction in the instruction set and access any memory location in the system. **The only way for the process to change from the user mode to the kernel mode is via an exception such as `a interrupt`,` a fault`, or `a trapping system call`.** 

Linux provides a mechanism called `/proc system` that allows user mode processes to access the content of kernel data structure.

### <a name="15">Context Switch : Hardware timer and kernel software</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

The context is the state that the kernel needs to restart a preempted process. It consist of

+ general-purpose register
+ the floating-pointer register
+ the program counter
+ user's stack
+ status register
+ kernel's stack
+ page table
+ a process table
+ a file table

<p align="center"> 
  <img src="./readme-pic/context_switch.png" alt="context_switch" />
</p>


<p align="center">Context Switch, the figure from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/14-ecf-procs.pdf">cmu-213 slide</a></p>

A context switch can occur in the following situation:

+ The kernel is executing a system call on behalf of the user. If the system call blocks because it is waiting for some event to occur, then the kernel can put the current process to sleep and switch to another process.
+ As a result of an interrupt.

### <a name="16">Process Control</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

#### <a name="17"> `exit()`</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

Terminate the process with an *exit status* of status.

#### <a name="18">`fork()`: </a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

A parent process creates a new running child process, where the newly created child process gets an identical copy from the parent process's user-level virtual address space(code, data segment, heap, shared libraries, user stack and its open file descriptors). However, the pid between child and parent are different. In other words, child process duplicates the context of their parent but has a separate address space, where both of them have their own private address space and any subsequent changes of the variable that parent or child makes are private and not reflected in the memory of the other process.

Also, the `fork()` function **called once but it returns twice**: once in the calling process(the parent); and once in the newly created child process. In parent, `fork()` returns the PID of the child, whereas `fork()` returns a value of 0 for the child, where both parent and child are run concurrently.

Furthermore, **the child inherits all the parent's open files**, and thus it can get the same output as their parent does for the same file.

When a process terminates for any reason, the kernel does not remove it from the system immediately. Instead, t**he process is kept around in a terminated state until it is reaped by its parent**, which called ***zombie***. When parent process terminates, the kernel arranges for the init process(PID = 1, launched at the system startup) to become the adopted parent of  any orphaned children.

#### <a name="19">`wait()`</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

A process waits for its children to terminate or stop by calling the `waitpid()` or `wait()`(simpler version) function. Parent reaps a child by calling the wait function In other words, it suspends current process un;l one of its children terminates and return value is the pid of the child process that terminated. 

#### <a name="20">`execve()`</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

Loading and running the executable object file *filename* with the argument list *argv* and the environment variable *envp*. `execve` returns to the calling program only if there is an error. In the most of case, `execve` **call once and never return**.

Furthermore  `execve` overwrites code, data and stack for the current process but retains PID, open files and signal context.

### <a name="21">Shell</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

Normally, if want to create a newly process, we often use `fork()` to create separate address space and use `execve()` to replace the context in that space.

A **shell** is an interactive application-level program that runs programs on behalf of the user. A shell performs a sequence of *read/evaluate* steps and then terminates. Specifically, the read step reads a command line from the user and the evaluate step parses the command line and runs programs on behalf of the user.

<p align="center"> 
  <img src="./readme-pic/linux_process_hierarchy.png" alt="linux_process_hierarchy" />
</p>


<p align="center">Linux Process Hierarchy, the figure from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/15-ecf-signals.pdf">cmu-213 slide</a></p>

### <a name="22">Signal : kernel software and application software</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

A signal is a small message that notifies a process that an event of some type has occurred in the system, where the signal happened in a higher-level software form of exceptional control flow that allows the process and the kernel to interrupt other processes. Each signal type corresponds to some kind of system event. Low-level hardware exceptions are processed by the kernel's exception handlers and would not normally be visible to user process. **Signal provide a mechanism for exposing the occurrence of such exception to user process**. **Only information in a signal is its ID** and the fact that it arrived.

#### <a name="23">Sending Signal</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

The kernel *sends(delivers)* a signal to a destination process by updating some state in the context of the destination process when:

+ the kernel has detected a system event.

+ A process has invoked the ***kill*** system call(`/bin/kill` in Unix) to explicitly request

  the kernel to **send a signal** to the destination process, where **`kill `system call does not kill any process** unless you pass the related signal(`SIGKILL`) to kill it.

 For example:

we can use `linux > /bin/kill -9 15213` to send signal number 9(`SIGKILL`) to process/process group 15213.

##### <a name="24">Process Group</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

Every process belongs to exactly one **process group**, which is identified by a positive integer **process group ID**. By default, a child process belongs to the same process group as its parent.

<p align="center"> 
  <img src="./readme-pic/process_group.png" alt="process_group" />
</p>


<p align="center">Process group, the figure from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/15-ecf-signals.pdf">cmu-213 slide</a></p>

#### <a name="25">Receiving a Signal</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

A destination process receives a signal when it is **forced by the kerne**l to react in some way to the delivery of the signal.

The process can either: **terminate the process**, **ignore** or **catch** the signal by executing a user-level function called a **signal handler**.

<p align="center"> 
  <img src="./readme-pic/receive_the_signal.png" alt="receive_the_signal" />
</p>


<p align="center">Receving the signal, the figure from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/15-ecf-signals.pdf">cmu-213 slide</a></p>

When the kernel switches a process p from kernel mode to user mode, it checks the set of unblocked pending signals(`pnb = pending & ~blocked`). If this set `pnb` is empty, then the kernel passes the control to the next instruction in the logical flow of p. Otherwise, the kernel will choose least nonzero bit `k `in `pnb` and force process p  to receive signal `k` and repeat this operation until `pnb` is zero.

Each signal type has a predefined ***default action***:

+ The process terminates.
+ The process terminates and dump core.
+ The process stops until restarted by a `SIGCONT` signal.
+ The process ignore the signal.

Note that **`signal` system call does not actually signal anything but modify the default action accociate with some signal.** The only exceptions are `SIGSTOP `and `SIGKILL`, whose default actions cannot be changed.

The signal function are change the action associated with a signal  `signum` in one of three ways:

+ If the handler is `SIG_IGN`, then signals of type `signum` are ignored.
+ If the handler is `SIG_DFL`, then the action of signals of type `signum` reverts to the default action.
+ Otherwise, handler is the address of a **user-defined function**, called a `signal handler`, that will be called whenever the process receives a signal of type `signum`. 
  + Changing the default action by **passing the address of a handler** to the `signal` function is known as ***installing the handler***. 
  + The **invocation of the handler** is called ***catching the signal***.
  + The **execution of the handler** is referred to as ***handling the signal***.

When a process catches a signal of type k, the handler installed for signal k is invoked with a single integer argument set to k. This argument allows **the same handler function to catch different types of signals**.

When the handler executes its *return* statement, control(usually) **passes back to the instruction in the control flow** where the process was interrupted by the receipt of the signal.

#### <a name="26">Pending and Blocked Signal</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

A signal that has been sent but not yet received is called a **pending signal**. At any point in time, there can be at most one pending signal of a particular type. **A pending signal is received at most once**. If a process has a pending signal of type k, then any subsequent signals of type k sent to that process are not queued; they are simply discard.

A process can selectively block the receipt of certain signals. When a signal is blocked, it can be delivered, but the resulting pending signal will not be received until the process unblock the signal. 

For each process, the kernel **maintains the set of pending signals** in the pending bit vector, and **the set of blocked signals** in the **blocked bit vector**. The kernel sets bit k in pending whenever a signal of type k is delivered and clears bit k in pending whenever a signal of type k is received.

In Linux, it provides **implicit** and **explicit** mechanisms for blocking signal: 

+ ***Implicit blocking mechanism***: By default, the kernel blocks any pending signals of the type currently being **processed by a handler**.
+ ***Explicit blocking mechanism***: Application can explicitly block/unblock selected signal using the `sigprocmask` function and its helpers.
  + *SIG_BLOCK*: add the signal in set to blocked(blocked = blocked | set).
  + *SIG_UNBLOCK*: Remove the signals in set from blocked (blocked = blocked & ~set).
  + *SIG_SETMASK*: blocked = set.
  + If `oldset `is non-NULL, the previous value of the blocked bit vector is stored in `oldset`.

#### <a name="27">Guidelines for Wri&ng Safe Handlers</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

+ G0: Keep your handlers as simple as possible
+ G1: Call only `async‐signal­‐safe functions` in your handlers
+ G2: Save and restore `errno `on entry and exit
+ G3: Protect accesses to shared data structures by **temporarily blocking all signals**.
+ G4: Declare global variables as `volatile`
+ G5: Declare global ﬂags as `volatile sig_atomic_t`
  + ﬂag: variable that is only read or wriien (e.g. ﬂag = 1, not ﬂag++)
  + Flag declared this way does not need to be protected like other globals

**Look aside**: the `async‐signal­‐safe functions` is one that can safely called from within signal handler. Specifically, **non-reentrant functions** are generally unsafe to call from a signal handler. [The below content is from this link](https://www.gnu.org/software/libc/manual/html_node/Nonreentrancy.html)

+ If a function uses a **static variable** or a **global variable**, or a **dynamically-allocated object** that it finds for itself, then it is non-reentrant and any two calls to the function can interfere.
+ If a function uses and **modifies an object that you supply**, then it is potentially non-reentrant; two calls can interfere if they use the same object.
+ On most systems, `malloc` and `free` are not reentrant, because they use a static data structure which records what memory blocks are free. As a result, no library functions that allocate or free memory are reentrant. This includes functions that allocate space to store a result.
  + The best way to avoid the need to allocate memory in a handler is to **allocate in advance space for signal handlers** to use.
  + The best way to avoid freeing memory in a handler is to f**lag or record the objects to be freed**, and **have the program check from time to time whether anything is waiting to be freed**. But this must be done with care, because **placing an object on a chain is not atomic**, and if it is interrupted by another signal handler that does the same thing, you could “lose” one of the objects.
+ Any function that **modifies `errno` is non-reentrant**, but you can correct for this: in the handler, **save the original value of `errno` and restore it before returning normally**. This prevents errors that occur within the signal handler from being confused with errors from system calls at the point the program is interrupted to run the handler.
  +  if you want to call in a handler a function that modifies a particular object in memory, you can make this safe by saving and restoring that object.
  +  **Note that `errno` is whenever a system call error occurs, and system call can have a variety of errors. `Errno `is how you figure out which one actually happened. In other words, `errno `is sent by a lot of different system calls, so if you're not going to use errno immediately after the call failed, you'd better to save the current errno in another variable.**
+ **Merely reading from a memory object is safe** provided that you can deal with any of the values that might appear in the object at a time when the signal can be delivered. Keep in mind that assignment to some data types requires more than one instruction, which means that the handler could run “in the middle of” an assignment to the variable if its type is not atomic.
+ **Merely writing into a memory object is safe as long as a sudden change in the value**, at any time when the handler might run, will not disturb anything.

## <a name="28">Solution</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

In this lab, all we need to do is to implement a simple shell `tsh.c`, where this file has already provided a code structure and some utility function.

**I highly recommend you to carefully read the shell examples in the second half of [this slide](https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/15-ecf-signals.pdf), which has some code examples in it.** **Also, please carefully read every comment above the to-do function.** **What's more, going through `tsh.c` is necessary and highly recommend.**

For those uility functions or system calls, I wrapped them into the wrapper function and put them into  `csapp.c`/`csapp.h`.

### <a name="29">Check the built-in function</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

In the shell, normally, the command divided by two types: 

1. built-in command.
2. non-built-in command.

In this lab, we have five built-in command:

1. `jobs `: list all the current running jobs
2. `fg`: revoke the sleep process and run it in the foreground
3. `bg`: revoke the sleep process and run it in the background
4. `quit/exit`: quit the shell
5. `&`: running the current job in the background.

Basically, we use `strcmp()` to do the comparison between our first input value(`argv[0]`) and the built-in command.

+ For `quit/exit` command, we simply call `exit(EXIT_SUCCESS)` to exit the current process.
+ For `fg` or `bg`, we call `do_bgfg(argv)` and return 1 to indicate the current command is the built-in command. We will discuss `do_bgfg` in detail later.
+ For `jobs`, we call `listjobs(jobs)` and return 1 to indicate this is the built-in command, where `listjobs(jobs)` has already implemented.

```c
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
```

In this lab, we also need to use 4 type of signal: **1. SIGCHLD**; **2. SIGTSTP/SIGSTP**; **3. SIGINT**; **4. SIGQUIT**, where the lab has already implemented the **SIGQUIT** and thus we need to implement all of the other three signal handlers.

### <a name="30">SIGINT handler:</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

The purpose of **SIGINT** is to tell computer interrupt the current process, where this behave is what system really do when typing the `ctrl+c` command.

```c
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
  	// send signal to process
    Kill(pid, SIGINT);
    errno = olderrno;
}
```

Note that for **slow systems call**(such as `read`,`wait` and `accept`) that potentially block the process for a long period of time, the programmer must include code that **manually restart** interrupted system calls.

### <a name="31">SIGTSTP handler: </a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

This signal is to stop typed at terminal, where it will hang up the current process but still exist in the job list of shell. When we type `fg` or `bg` command, the hanging up process will be awaked to the foreground/background.

```c
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
```

### <a name="32">SIGCHLD handler</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

`SIGCHLD`: This signal is used to change the child process's state. When parent create a child process, instead of waiting for child process terminated, parent will do the other work until child process send the `SIGCHLD` signal(the kernel send the `SIGCHLD` signal to their parent when one of its child process terminates or stops). After parent received/caches the signal, it will recap this child process.

Note that: `waitpid` system call suspends execution of the calling process until a child specified by pid argument has changed state.  **By default, waitpid() waits only for terminated children**, but this behavior is modifiable via the options argument

```c
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
    // WNOHANG: return immediately if no child has exited.
    // WUNTRACED: also return if a child has stopped (but not traced via ptrace(2)).
    // Status for traced children which have stopped is provided even if this option is not specified.
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
```

### <a name="33">Waitfg</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

In here, I provide two ways to implement it, where one is use `volatile sig_atomic_t flag`, the other(the comment lines) is not. The purpose of this function is to block the current process until it is no longer the foreground process(usually it is to wait the child function finished by receiving SIGCHLD signal). The core signal action in here is to use `sigsuspend`, no matter which implementation above we choose, this action is not changed.

```c
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
```

### <a name="34">do_fgbg</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

This function is to implement the way of executing foreground/background job. The logic is relatively simple, where we get the current job id(jid) first, and then use this jib to find the pid of the current process; or get the pid directly. After that, we need to send the `SIGCONT` to revoke the suspended process(the purpose of bg/fg command is to awake the suspend process). Finally, we will check the current job need to be run at foreground or background by checking the first argurment of the input command(`argv[0]`), if it is the background,  just change the flag of the correspond job to `BG`, change to `FG` and call `waitfg` otherwise.

```c
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
```

### <a name="35">Eval:</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

This is the core function of shell, where when shell execute a new command, it will create a child process and take over rest of  task. Also the main point of this function is how to deal with multiply signal, where the signal is not queued and we cannot count the signal. Instead, we will use the signal mask to block signal before its relative action running and unblock it after the job finished by using  `sigprocmask` to make sure every signal relatead action is atomic.

```cpp
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

```

## <a name="36">Test Our Shell</a><a style="float:right;text-decoration:none;" href="#index">[Top]</a>

The way to test our shell is to use `make testxx` command, where tests include `test01 ~ test16`, where each test correspond a `trace file(tracexx.txt)`. You should make sure each test can finish by itselves without any exception. Moreover, you can also compare with your test output with `tshrf.out`, where this file include the correct answer for each file.



Shell Lab Finished.
