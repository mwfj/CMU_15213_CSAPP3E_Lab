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



### Cache Memory Read Background

Before staring to write the code, we need to figure out **the cache line structure** and how it works first, where it should from the Book([CSApp3E](https://csapp.cs.cmu.edu/)) or [slides](https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/schedule.html).

Just like the book talks about, the cache memory structure as figure show below:

![cache_memory_structure](./readme-pic/cache_memory_structure.png)

<p align="center">Cache line structure from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/11-memory-hierarchy.pdf">cmu-213 slide</a></p>

Looking at the picture above now, we need to clearly distinguish two different types of address before we move on. The left side is `structure of cache memory`; whereas the right side the `structure of address of word`.  `s bits` is the index number of the whole cache memory, which help us to location the position of data we looking for by set sizze. Furthermore, if we look at these structures closely, we will find that there have one common section between these two structure : **tag bit**, where it help us to make sure whether the current cache line is the one we looking for. Actually, there also has a connection between **cache block** and **block offset**, where it is that block offset just like an index of an array to point out that position of the first byte of the data we looking for in cache block. In the final, `valid bit` is to tell us whether current cache line in the selected set is **meaningful**.

```
/**
 * Structure for block in Cache line:
 *      +-----------+----------+---------------+
 *      + Valid Bit +  Tag Bit +  Cache Block  + 
 *      +-----------+----------+---------------+    
 *
 *
 * Address of word that CPU send to Cache: 64bit 
 *      +-----------+------------+---------------+
 *      + Tag Bit   +  Set Index +  Block Offset +
 *      +-----------+------------+---------------+
**/ 
```



Then, we need to figure out how cache hardware implements read: 

1. Program executes instructions first, it reference some word to read in memory.
2. CPU will send the address to cache and asks the cache to return the word(the data it looking for) at that address
3. Cache will take that address and divides the address into a number of regions, which determined by the organization of the cache, where it characterized by tuple(S.E.B).

Generally, the size(capacity) of cache `C = S x E x B` **(valid bits and tag bits are not include)**

+ `s set index` bits form a index into the array of S sets, where the first is 0, the second is 1 and so on.

+ Once we know which set the word must be contained in, **tag bits** `t` tell us which line(if any) in the set contains the word.

+ A line in the set contains the word if and only if the `valid bit` is set and `the tag bit` in the line **match the tag bit** in the addresses.

+ Once we have **located the line** identified by **the tag** in the set identified by **the set index**, then the `b block offset bits` gives us the offset of the word in the **B-byte data block**. 

  

<p align="center"><strong>Fundamental Parameters Table</strong></p>

| Parameter   | Description                                       |
| ----------- | ------------------------------------------------- |
| S = 2ˢ      | Number of **sets**                                |
| E           | Number of **lines per set**                       |
| B = 2ᵇ      | Block size (bytes)                                |
| m = log₂(M) | Number of **physical (main memory) address bits** |

<p align="center">This table is from the book <a href = "http://csapp.cs.cmu.edu/3e/home.html">CS:APP3e</a>  chapter 6</p>



<p align="center"><strong>Derived quantities Table</strong></p>

| Parameter       | Description                                                  |
| --------------- | ------------------------------------------------------------ |
| M = 2ᵐ          | **Maxinum** number of **unique** memory addresses            |
| s = log₂(S)     | Number of **set index bits**                                 |
| b = log₂(B)     | Number of **block offset bits**                              |
| t = m - (s + b) | Number of **tag bits**                                       |
| C  = B x E x S  | **Cache size(bytes)**, not including overhead such as the valid and tag bits |

<p align="center">This table is from the book <a href = "http://csapp.cs.cmu.edu/3e/home.html">CS:APP3e</a>  chapter 6</p>



### Simple Example : Direct-Mapped Caches

A cache line with **exactly one line per set(E = 1)** is known as a direct-mapped caches. We use this simple structure of go through the process of cache read.

![direct-mapped_cache](./readme-pic/direct_mapped_cache.png?lastModify=1634221687)

<p align="center">Direct-mapped cache from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/11-memory-hierarchy.pdf">cmu-213 slide</a></p>

The process that a cache goes through of determinging whether a request is a hit or miss and then extracting word consists of three steps

+ ***set selection***:

  In this section, the cache extracts that **s set** index bits from the middle of the address for `target word ω`, where these bits interprets as an **unsigned integer** that corresponds to a set number. In other words, if we think of the cache as a one-dimensional array of sets, then the set index bits form an index into this array.

  ![set_selection](./readme-pic/set_selection.jpg)

  <p align="center">This figure is from the book <a href = "http://csapp.cs.cmu.edu/3e/home.html">CS:APP3e</a>  chapter 6</p>

+ ***line matching***:

  After making sure that the word ω we looking for is located at **set i**, we start to look for the exact position that the data located by comparing the `tag bit` and checking the `valid bit`. Specifically,  the valid bit indicate whether the current cache line in selected set  is meanning for or not and tag bit help to us to check whether the current cache line is right one.

  **The `cache hit` occurs only if the valid bit is 1 and the tag bit matches.** Otherwise, we would have a `cache miss`.

+ ***word extraction***

  Once we have a hit, we know that target word ω is somewhere in this block. All the things we need to do is to find the start poistion of the data we looking for in this block. To get this purpose, we need to use the `block offset` from the word address, where it provides us with the offset of the first byte in the desired word ω. Simliar to our view of a cache as an array of lines, we also can regards the data block as an array, and the block offset is the index number of that array.

  ![set_selection](./readme-pic/line_matching.jpg)

<p align="center">This figure is from the book <a href = "http://csapp.cs.cmu.edu/3e/home.html">CS:APP3e</a>  chapter 6</p>

+ Unfortunately, if we encounter a cache miss, then we need to retrieve the requested block from the next level in the memory hierarchy and store the new block in one of the cache lines of the set indicated by the set index bits. In general, if the current set is full of valid cache line, then one of the existing lines must be evicted.



## Part A: Writing a Cache Simulator

### Problem Description

**This section mainly focus on the cache read.**

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

### My Solution

Just like the related content in the previous section, the cache line structure in our code should be like this :

```c
/**
 * Structure for block in Cache line:
 *      +-----------+----------+---------------+
 *      + Valid Bit +  Tag Bit +  Cache Block  + 
 *      +-----------+----------+---------------+    
 *
 *
 * Address of word that CPU send to Cache: 64bit 
 *      +-----------+------------+---------------+
 *      + Tag Bit   +  Set Index +  Block Offset +
 *      +-----------+------------+---------------+
**/ 

// the structure of word address in cache
typedef struct cache_line{
    uint64_t tag; // used for 64 bit
  	// Because we use LRU as the evict rules
    // we need to record the current cache create time
    uint64_t time;
    int valid; // valid bit
    // int block; // block offset(unused in this simulator)
}cache_line_t;

typedef cache_line_t *cache_line_ptr;
```

