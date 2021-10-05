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

