# Lab 4 Cache Lab

### Updating ...

## Little Background Review:

Basically, in the memory syetem, there have two types of chips from the memory system:  **Dynamic Random-Access Memory(DRAM)** and **Static Random-Access Memory(SRAM)**, where **main memory** is composed of DRAM and **cache memory** is composed of SRAM. The reason they use different types of chips is that SRAM is more stable by disturbance and faster but cannot make dense and expensive, whereas DRAM can be made more densive and cheaper but more sensitive by a noise like the light ray and need to refresh. Furthermore, various sources of leakage current cause a DRAM cell to lose its charge within a time period around 10 to 100 millisecond, and thus the memory system must periodically refresh every bit of memory by reading it out and then rewriting it. 

|          | Transistors per bit | Relative Access Time | Persistent? | Relative Cost | Sensitive? | Applications                  |
| :------: | :-----------------: | :------------------: | :---------: | :-----------: | :--------: | ----------------------------- |
| **SRAM** |          6          |          1x          |     Yes     |     1000x     |     No     | Cache Memory                  |
| **DRAM** |          2          |         10x          |     No      |      1x       |    Yes     | Main Memory, <br>frame buffer |

Due to the speed gap between CPU, SRAM (Cache line) and DRAM (Main Memory), and even persistent devices (such as disk drives), we use a principle called ***locality*** to try to bridge the process-memory gap.

At **hardware level**, the principle of locality allows computer designers to speed up main memory accesses by introducing small fast memories to hold blocks of **recently referenced instructions and data items.**

At **operating system level**, the principle of locality allows the system to use main memory as cache of the most recently referenced chunks of the virutal address space( same as locality between disk and main memory).

Due to the **memory hierarchy** conception, the storage devices get slower, cheaper, and larger as we move from higher to lower levels, where the highest level(L0) can be accesses by CPU in a single clock cycle, whereas the next level SRAM cache can be taken a few clock cycles(see the table below). Furthermore, the central idea of a memory hierarchy is that for each `k`, the faster and smaller storage device at level `k `serves **as a cache for the larger and slower storage device at the level** `k+1`. In other words, each level in the hierarchy caches data objects from the next lower level.

![mem_hierarchy](./readme-pic/memory_hierarchy.png)

In general, devices lower in the hierarchy(further from the CPU) have **longer access times**, and thus tend to use larger block sizes in order to **amortize** these longer access times.

![cache_concept](./readme-pic/cache_concept.png)

<p align="center">These figures are from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/11-memory-hierarchy.pdf">cmu-213 slide</a></p>

|      Type      |      What cached      |     where cached     | Latency(clock cycles) |     Managed by      |
| :------------: | :-------------------: | :------------------: | :-------------------: | :-----------------: |
|  CPU register  | 4-byte or 8-byte word | On-chip CPU register |           0           |      Compiler       |
|      TLB       |  Address translation  |     On-chip TLB      |           0           |    Hardware MMU     |
|    L1 Cache    |    64-byte blocks     |   On-chip L1 cache   |           4           |      Hardware       |
|    L2 Cache    |    64-byte blocks     |   On-chip L2 cache   |          10           |      Hardware       |
|    L3 Cache    |    64-byte blocks     |   On-chip L3 cache   |          50           |      Hardware       |
| Virtual Memory |      4-KB pages       |     Main memory      |          200          |    Hardware + OS    |
|  Buffer Cache  |    Parts of files     |     Main memory      |          200          |         OS          |
|   Disk Cache   |     Disk sectors      |   Disk controller    |        100,000        | Controller firmware |
| Network Cache  |    Parts of files     |      Local disk      |      10,000,000       |     NFS client      |
| Browser Cache  |       Web pages       |      Local disk      |      10,000,000       |     Web browser     |
|   Web Cache    |       Web pages       | Remote server disks  |     1,000,000,000     |  Web proxy server   |

<p align="center">This table is from the book <a href = "http://csapp.cs.cmu.edu/3e/home.html">CS:APP3e</a>  chapter 6</p>

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

Just like the book talks about,