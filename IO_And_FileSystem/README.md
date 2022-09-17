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

The kernel returns ***file descriptor***, that identifies the file in all subsequent operations on the file. 

+ The kernel keeps track of all information about the open file. 
+ The application only keeps track of the descriptor

Each process created by a Linux shell begins life

fd = open(pathname, flags, mode) opens the file identified by pathname, returning a file descriptor used to refer to the open file in subsequent calls.