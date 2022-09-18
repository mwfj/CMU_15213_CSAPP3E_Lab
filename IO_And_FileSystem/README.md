# File System & System I/O

## File

Input/Output(I/O) is the process of **copying data between main memory and external device** such as disk drives, terminals and networks.

+ Input operation copies data from an I/O device to main memory;
+ Output operation copies data from memory to a device.

In linux, ***file*** is a sequence of m bytes: `B0, B1, ... , B_k, ..., B_(m-1)`.

Each Linux file has a type that indicates its role in the system:

+ A ***regular file*** contains arbitrary data. Application programs often distinguish between

  + **text files**: each line is a sequence of characters. A Linux text file consist of a sequence of *text lines*, where each line is a sequence of characters terminated by a `newline character('\n')`. The newline character is the same as the ASCII line feed character(LF) and has numeric value of `0x0a`;
  + **binary files**: everything except text file

  To kernel, there no difference between text and binary files/

+ A ***directory*** is a file consisting of an array of links, where each links maps a filename to a file, which may be another directory;

  Each directories contains at least two entries:

  + `.`(dot): the link to the directory itself
  + `..`(dot-dot): the link to its parent directory in the directory hierarchy.

  You can create a directory with the `mkdir` command; view ites content with `ls`, and delete it with `rmdir or rm -rf`

  <p align="center"> <img src="./pic/portion_of_linux_hierarchy.png" alt="cow" style="zoom:100%;"/> </p>

  <p align="center">Portion of the Linux directory hierarchy <a href = "http://csapp.cs.cmu.edu/3e/home.html">CS:APP3e</a>  chapter 10</p>

+ A ***socket*** is a file that is used to communicate with another process cross a network.

All I/O device, such as networks, disks, and terminals, are modeled as files, and all input and output is performanced by reading and writing the appropriate files:

+ `/dev/sda2`(`/user` disk partition)
+ `/dev/tty2`(terminal)
+ `/boot/vmlinuz-3.13.0-55-generic`(kernel image)
+ `/proc` (kernel data structures)

This elegant mapping of files allows the Linux kernel to export a simple, low-level application interface, known as Unix I/O, that enables all input and output to be performed in a uniform and consisent way:

### Opening Files `open()`

An application announces its intention to access an I/O by asking the kernel to open the corresponding file.

The `open` function converts a `filename` to a file descriptor and returns the descriptor number, the descriptor returned always the smallest descriptor that is not currently open in the process, where `fd = open(pathname, flags, mode)` opens the file identified by pathname, returning a file descriptor used to refer to the open file in subsequent calls. (If the `pathname` is symbolic link, it is dereferenced)

The kernel returns ***file descriptor***, that identifies the file in all subsequent operations on the file. 

+ The kernel keeps track of all information about the open file. 
+ The application only keeps track of the descriptor

```c
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int open(char *filename, int flags, mode_t mode); // return new descriptor if OK, -1 otherwise
```

Each process created by a Linux shell begins life with three open files:

+ standard input (descriptor 0) (STDIN_ FILENO in `<unistd.h>`)
+ standard output (descriptor 1) (STDOUT_FILENO in `<unistd.h>`)
+ standard error (descriptor 2) (STDERR_FILENO in `<unistd.h>`)

#### Flags

The `flags` argument can also be bit-wise  **ORed**(|) in flags with one or more bits masks that provide additional instructions

The table below divided into the following groups:

+ ***File access mode flags***: There are the flags like O_RDONLY, O_WRONLY, O_RDWR flags, where they can be retrieved using `fcntl()` F_GETFL operation.

+ ***File creation flags:*** These are the flags shown in the second part of the table below, where they control various aspects of the behavior of the `open()` call, as well as options for subsequent I/O operations. These flags cannot be retrieved or changed
+ ***Open file status flags:*** There are the remaining flags in the table below, where they can be retrieved and modified using the `fcntl()` F_GETFL and F_SETFL operations.

<p align="center"> <img src="./pic/flag_bit_in_open.png" alt="cow" style="zoom:100%;"/> </p>

<p align="center">Values of the flags argument of open() <a href = "https://man7.org/tlpi/">The Linux programming interface</a>  chapter 4</p>

#### Mode Argument

The mode(`st_mode `in `stat `structure) argument specifies the access permission bit of new files. As part of its context, each process has a `umask` that is set by calling the unmask function. The first 3 of these bits are special bits known as the **set-user-ID**, **set-group-ID**, and **sticky bits**(labeled U, G, and T commonly)

+ **set-user-ID program:** A set-user-ID program allows a process to **gain privileges it would not normally have**, by **setting the process's effective user ID** to the same value as the user ID(owner) of the executable file; When a set-user-ID program is run, **the kernel sets the effective user ID of the process to be the same the user ID** of the executable file;

+ **set-group-ID program:** A set-group-ID performs the analogous task for the process's effective group ID. (**effective group ID** and **effective userID** are used to determine the permission granted to a process, normally they are the same value); When a set-group-ID program is run, **the kernel sets the effective user ID of the process to be the same the group ID** of the executable file;

  For the perspective of stikcy bit, set-group-ID mainly serve two purpose:

  + controlling the group ownership of new files created in a directory mounted with the `nogrpid `option
  + enabling mandatory locking on a file

+ **set-user-ID program and set-group-ID program** can also designed to change the effective IDs of a process to something other than `root`

+ **set-user-ID bit** and **set-group-ID bit** are set using the `chmod` command 

  + `chmod u+s`: turn on set-user-ID permission bit;
  + `chmod g+s`: turn on set-group-ID permission bit.

+ **sticky bit**: 

  + *On older UNIX implementations*, the sticky bit was provided as a way of **making commonly used programs run faster**.

    +  **If the sticky bit was set on a program file**, then the first time the program was executed, a copy of the program text was **saved in the swap area**—thus **it sticks in swap, and loads faster** on subsequent executions.

  + *In modern UNIX implementations (including Linux)*:

    + **For directories**, the sticky bit acts as the **restricted deletion flag**. Setting this bit on a directory means that an **unprivileged process** can **unlink** (`unlink()`,` rmdir()`) and **rename** (`rename()`) files in the directory only if it has write permission on the directory and owns either the file or the directory.

      This makes it possible to create a directory that is **shared by many users**, who can each **create and delete their own files in the directory but can’t delete files owned by other users**. The sticky permission bit is commonly set on the `/tmp` directory for this reason.

    + **A file’s sticky permission bit** is set via the chmod command (`chmod +t file`) or via the `chmod()` system call. If the sticky bit for a file is set, `ls –l` shows a lowercase or uppercase letter T in the other-execute permission field, depending on whether the other-execute permission bit is on or off, as in the following:

      ```shell
      $ touch tfile 
      $ ls -l tfile
      -rw-r--r-- 1 mtk  users   0 Jun 23 14:44 tfile
      $ chmod +t tfile 
      $ ls -l tfile
      -rw-r--r-T 1 mtk  users   0 Jun 23 14:44 tfile
      $ chmod o+x tfile 
      $ ls -l tfile
      -rw-r--r-t 1 mtk  users   0 Jun 23 14:44 tfile
      ```

The **remaining 9 bits** form the mask defining the permission that are granted to various categories of users accessing the file. The file permission maks divideds the world into three categories:

+ ***Owner***(also known as ***user***): The permissions granted the owner of the file;
+ ***Group***: The permission granted to user who are members of the file's group
+ ***Other***: The permission granted to everyone else

Three permission may be granted to each user category:

+ ***Read***: The content of the file may be read
+ ***Write***: The content of the file may be changed
+ ***Execute***: The file may be executerd(i.e. it is a program or a script). In order to execute a script file, **both read and execute permission are required**.

The permission and ownership of a file can be viewed using the command `ls -l`, as in the following example:

<p align="center"> <img src="./pic/file_permission_in_ls_command.png" alt="cow" style="zoom:100%;"/> </p>

The `<sys/stat.h>` header file defines constants that can be **ANDed (&)** with `st_mode `of the `stat `structure, in order to **check whether particular permission bits are set.** (These constants are also defined via the inclusion of `<fcntl.h>`, which prototypes the `open()` system call.)

| Constant                            | Octal Value               | Permission Bit                                 | Description                                                  |
| :---------------------------------- | :------------------------ | ---------------------------------------------- | :----------------------------------------------------------- |
| S_ISUID <br />S_ISGID <br />S_ISVTX | 04000<br/>02000<br/>01000 | Set-user-ID<br />Set-group-ID<br />Sticky      | This is the set-user-ID on execute bit<br />This is the set-group-ID on execute bit<br />This is the *sticky* bit. |
| S_IRUSR<br />S_IWUSR<br />S_IXUSR   | 0400<br/>0200<br/>0100    | User-read<br />User-write<br />User-execute    | User (owner) can read this ﬁle<br />User (owner) can write this ﬁle<br />User (owner) can execute this ﬁle |
| S_IRGRP<br />S_IWGRP<br />S_IXGRP   | 040<br/>020<br/>010       | Group-read<br />Group-write<br />Group-execute | Members of the owner’s group can read this ﬁle<br />Members of the owner’s group can write this ﬁle<br />Members of the owner’s group can execute this ﬁle |
| S_IROTH<br />S_IWOTH<br />S_IXOTH   | 04<br/>02<br/>01          | Other-read<br />Other-write<br />Other-execute | Others (anyone) can read this ﬁle<br />Others (anyone) can write this ﬁle<br />Others (anyone) can execute this ﬁle |

In addition to the mode bit shown above, **three constants** are defined to equate to **masks for all three permissions for each of the categories owner, group, and other**: `S_IRWXU (0700)`, `S_IRWXG (070)`, and `S_IRWXO (07)`.

When `open()` is used to create a new file, the *mode* bit-mask argument specifies the permissions to be placed on the file. If the `open()` call doesn't specify `O_CREAT`, mode can be ommitted

```c
/* Open existing file for reading */
fd = open("startup", O_RDONLY);
if (fd == -1)
	errExit("open"); 
/* 
 * Open new or existing file for reading and writing, truncating to zero bytes; 
 * file permissions read+write for owner, nothing for all others 
 */ 
fd = open("myfile", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

if (fd == -1)
  errExit("open"); /* Open new or existing file for writing; writes should always append to end of file */ 

fd = open("w.log", O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, S_IRUSR | S_IWUSR);

if (fd == -1)
  errExit("open");
```

#### `umake( or system call umask() )`

The ***umask*** is a process attribute that specifies which **permission bits should always be *turned off* when new files or directories are created by the process**. Often, **a process just uses the umask it inherits from its parent shell**, with the (usually desirable) consequence that the **user can control the umask of programs** executed from the shell using the shell built-in command umask, which changes the umask of the shell process.

The initialization files for most shells **set the default umask to the octal value** `022` (`----w--w-`). This value specifies that write permission should always be turned off for group and other.

The `umask()` system call changes a process’s umask to the value specified in ***mask***.

```c
#include <sys/stat.h>
mode_t umask(mode_t mask);
```

When a process creates a new file by calling the open function with some mode argument, then the access permission bits of the file are set to `mode & ~umask`. For example, suppose we  are given the following  default values for `mode` and  `umask`:

```c
#define DEF_MODE  S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH 
#define DEF_UMASK S_IWGRP|S_IWOTH
```

Then the following code fragment creates a new file in which the owner of the file has read and write permission, and all other users have read permissions:

```c
umask(DEF_UMAKE)
fd = Open("foo.txt", O_CREAT|O_TRUNC|O_WRONLY, DEF_MODE);
```

**A call to `umask()` is always successful, and returns the previous umask.**

