# Malloc Lab Report
Updating ...

## Background

### Virtual Memory

Virtual memory provides three important capabilities:

1. It uses main memory efficiently by **treating it as a cache** for an address space stored on disk, **keeping only the active areas in main memory** and transferring data back and forth between disk and memory as needed.
2. It simplifies memory management by providing each process with a **uniform address space**.
3. It **protects** the address space of each process from **corruption by other processes**.

With virtual memory, the CPU accesses main memory by generating a virtual address(VA), which is converted by the appropriate physical address before being sent to main memory. The task of **converting a virtual address to physical address** is known as ***address translation***.

Dedicated hardware on the CPU chip called the **memory management unit(MMU)** translates virtual addresses on the fly, using a lookup table stored in main memory whose content are managed by the operating system.

![virtual_addressing](./pic/virtual_addressing.png)

<p align="center">The address translation, the figure from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/17-vm-concepts.pdf">cmu-213 slide</a></p>

#### Address Spaces

An address space it an ordered set of nonnegative integer addresses `{0, 1, 2, ...}`.

If the integer in the address space are consecutive, then we say that it is a ***linear address space***.

In a system with virtual memory, the CPU generates virtual addresses from an addpress space of `𝑵 = 2ⁿ` called the ***virtual address space***: `{0, 1, 2, ..., 𝑵 - 1}`, where the size of an address space is characterized by the number of bits that are needed to represent the largest address. For example, a virtual address space with `𝑵 = 2ⁿ`  is called **n-bit address space**(such as 32-bit or 64-bit address space).

A system also has a physical address space that corresponds to the 𝑴 bytes of physical memory in the system: `{0, 1, 2, ..., 𝑴 - 1}`, where **𝑴 is not required to be a power of 2**.



Conceptually, a virtual memory is organized as **an array of N contiguous by-size cells stored on disk**, where each byte has a unique virtual address that serves as an index into the array. **The contents of the array on disk are cached in main memory.** 

As with any other cache in the memory hierarchy, the data on disk(the lower level) is partitioned into blocks that serve as the transfer units between disk and the main memory(the upper level). VM systems handle this by partioning the virtual memory into fixed-size blocks called ***virtual pages(VPs)***. Each virual page is `𝑷 = 2ᵖ` bytes in size.

Similarly, physical memory is partitioned into ***physical pages(PPs)***,also 𝑷 bytes in size. (Physical pages are also referred as ***page frames***)

At any point in time, the set of virtual pages is partitioned into **three disjoint subset**:

+ `Unallocated`. Pages that **have not yet been allocated(or created)** by the VM system. Unallocated blocks do not have any data associated with them, and thus do not occupy any space on disk.
+ `Cached`: Allocated pages that are **currently cached** in physical memory.
+ `Uncached`: Allocated pages that are **not cached** in physical memory.

![vp_pp](./pic/vp_pp.png)

<p align="center">The mapping between VPs -> PPs, the figure from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/17-vm-concepts.pdf">cmu-213 slide</a></p>

Because of the large miss penalty and the expense of accessing the first byte(disk is about 100,000 times slower than a DREAM), virtual page tend to be large－typically 4KB to 2MB. Due to the large miss penalty, **DRAM caches are fully assoicative; that is, any virtual page can be placed in any physical page** and also it requires a “large” mapping function.

Finally, because of the large access time of disk, **DRAM caches always use write-back instead of write-through**, where virual memory system try to **defer writing** anything back to the disk as long as possible.

#### Page Tables

As with any cache, the VM system must have some way to determine if a virtual page is cached somewhere in DRAM. If so, the system must determine which page it is cached in. If there is a miss, the system must determine where the virtual page is stored on disk. select a victim page in physical memory, and copy the virtual page from disk to DRAM, replacing the victim page. 

These capabilities are provided by a combination of

+ Operating system software;
+ Address translation in the ***MMU(Memory Management Unit)***;
+ A data structure stored in physical memory known as a ***page table*** that maps virtual pages to physical pages. The address translation hardware reads the page table each time it converts a virtual address to a physical address.

**The operating system is responsible for maintaining the contents of the page table and transferring pages back and forth between disk and DRAM.**

A page table is an array of ***page table entry(PTE)***. **Each page in the virtual address space has a PTE at a fixed offset in the page table.** 

![page_table](./pic/page_table.png)

<p align="center">Page Table, the figure from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/17-vm-concepts.pdf">cmu-213 slide</a></p>

In the picture above:

+ valid bit is set, that means the current page is cached in DRAM
+ valid bit is not set:
  + a null address indicates that the virtual page has not yet been allocated.
  + otherwise, the address points to the start of the virtual page on disk(not cached).

**Page Hit:** reference to VM word that is in physical memory(DRAM cache hit).

**Page Faults:** In virtual  memory parlance, **a DRAM cache miss known as a page fault**, where it reference to VM word that is **not in physical memory**. Page fault causes the transfer of chunk of code in the kernel called ***page fault handler***, which select a victm page.  

+ When CPU reference a word VP but that word has not in the memory, **a page fault exception** will be triggered and that exception then invoke a **page fault exception** handler in the kernel.
+ Next, the kernel copies **VP(virtual page) from disk to PP(physical page)** in memory, **updates PTE(page table entry)** and then returns.
+ When the handler returns, it restarts the faulting instruction, which **resends the faulting virtual address to the address translation hardware(MMU)**.
+ In the final, MMU check PTE again and fetch it.

For the terminology perspective, blocks in DRAM(main memory) and SRAM(cache memory) is known as ***pages***. 

The activity of transferring a page between disk and memory is known as ***swapping*** or ***paging***.

Pages are ***swapped in***(paged in) from disk to DRAM, and ***swapped out***(paged out) from DRAM to disk.

The strategy of **waiting until the last moment** to swap in a page, when a miss occurs, is known as ***demand paging***.



Although the total number of distinct pages that programs reference during an entrie run might exceed the total size of physical memory, **the principle of locality** promises that any any point in time they will tend to work on a **smaller set** of ***active pages*** known as the ***working set*** or ***resident set***.

If the working set size exceeds the size of physical memory, then the program can produce an unfortunate situation known as thrashing, where pages are swappe in and out continuously.

#### VM as a tool for memory management

In fact, operating systems provide **a separate page table**, and thus **a separate virtual address space**, for each process.

Notice that **multiple virtual page can be mapped to the same shared physical page**.

The combination of demand pageing and separate virtual address space has a profound impact on the way that memory is used and managed in a system. In paricular, **VM simplifies linking and loading, the sharing of code and data, and allocating memory to applications**.

+ *Simplifying Linking*: a separate address space allows each process to use the same basic format for its memory image, regardless of where the code and data actually reside in physical memory. 

  For example, 64-bit address space in Linux:

  + **the code segment always starts at virtual address 0x400000.** 
  + The data segment follows the code segment after a suitable alignment gap.
  + The stack occupies the highest portion of the user process address space and grows downward.



