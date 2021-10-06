# Lab 4 Cache Lab

### Table of Content

## Part A: Writing a Cache Simulator

### Problem Description

In Part A you will write a cache simulator in `csim.c `that takes a `valgrind `memory trace as input, simulates the hit/miss behavior of a cache memory on this trace, and outputs the total number of hits, misses, and evictions.

Valgrind memory traces have the following form:

```
I 0400d7d4,8
 M 0421c7f0,4
 L 04f6b868,8
 S 7ff0005c8,8
```

Each line denotes one or two memory accesses. The format of each line is `[space]operation address,size`

We have provided you with the binary executable of a reference cache simulator, called `csim-ref`, that simulates the behavior of a cache with arbitrary size and associativity on a valgrind trace ﬁle. It uses the **LRU (least-recently used)** replacement policy when choosing which cache line to evict.

Your job for Part A is to **ﬁll in the `csim.c` ﬁle so that it takes the same command line arguments and produces the identical output as the reference simulator**. Notice that this ﬁle is almost completely empty. You’ll need to write it from scratch.

+ Your simulator must work correctly for arbitrary `s`, `E`, and `b`. This means that you will need to allocate storage for your simulator’s data structures using the malloc function. Type “man malloc” for information about this function.
+ For this lab, we are interested only in **data cache performance**, so your simulator should **ignore all instruction cache accesses** (lines starting with “I”). Recall that valgrind always puts “I” in the ﬁrst column (with no preceding space), and “M”, “L”, and “S” in the second column (with a preceding space). This may help you parse the trace.

### Solution

First of all, we need to figure out **the cache line structure** and how it works, where it should from the Book([CSApp3E](https://csapp.cs.cmu.edu/)) or [slides](https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/schedule.html).

Basically, in the memory syetem, there have two types of chips from the memory system:  **Dynamic Random-Access Memory(DRAM)** and **Static Random-Access Memory(SRAM)**, where **main memory** is composed of DRAM and **cache memory** is composed of SRAM. The reason they use different types of chips is that SRAM is more stable by disturbance and faster but cannot make intense and expensive, whereas DRAM can be made more intensive and cheaper but more sensitive by a noise like the light ray and need to refresh around 10 to 100 millisecond.

|          | Transistors per bit | Relative Access Time | Persistent? | Sensitive | Relative Cost | Applications                  |
| :------: | :-----------------: | :------------------: | :---------: | :-------: | :-----------: | ----------------------------- |
| **SRAM** |          6          |          1x          |     Yes     |    No     |     1000x     | Cache Memory                  |
| **DRAM** |          2          |         10x          |     No      |    Yes    |      1x       | Main Memory, <br>frame buffer |

