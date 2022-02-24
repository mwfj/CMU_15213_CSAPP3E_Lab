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
+ Address translation in the ***MMU(Memory Management Unit)***, where **MMU** converts a logical address through a hardware call ***segmentation unit*** and then another hardware called ***paging unit***, which converts linear address into physical address.
+ A data structure stored in physical memory known as a ***page table*** that maps virtual pages to physical pages. The address translation hardware reads the page table each time it converts a virtual address to a physical address.

![mmu_structure](./pic/mmu_structure.jpeg)

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

#### VM as a Tool for Memory Management

In fact, operating systems provide **a separate page table**, and thus **a separate virtual address space**, for each process.

Notice that **multiple virtual page can be mapped to the same shared physical page**.

The combination of demand pageing and separate virtual address space has a profound impact on the way that memory is used and managed in a system. In paricular, **VM simplifies linking and loading, the sharing of code and data, and allocating memory to applications**.

+ ***Simplifying Linking***: a separate address space allows each process to use the same basic format for its memory image, regardless of where the code and data actually reside in physical memory. 

  For example, 64-bit address space in Linux:

  + **the code segment always starts at virtual address 0x400000.** 
  + The data segment follows the code segment after a suitable alignment gap.
  + The stack occupies the highest portion of the user process address space and grows downward.

+ ***Simplifying loading***: To load `.text` and `.data` sections of an object file into a **newly created process**, the Linux loader(`execve`) allocates virtual pages for the code and data segments, **marks them as invalid**(i.e. not cached) and **points their page table entries to the appropriate locations** in the object file. 

  The interesting point is that the **loader never actually copies any data from disk into memory**. 

  The data are paged in automatically and on demand by the virtual memory system the first time each page is referenced, either by **the CPU** when it fetches an instruction or by an **executing instruction** when it references a memory location.

  This notion of mapping a set of contiguous virtual pages to an arbitrary locaiton in an arbitrary file is known as ***memory mapping***. Linux provides a system call called `mmap `that allows application programs to do their own memory mapping.

  ![mem_layout](./pic/mem_layout.png)

  <p align="center">Memory layout, the figure from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/17-vm-concepts.pdf">cmu-213 slide</a></p>

+ ***Simplifying sharing***: Separate address spaces provide the operating system with a consistent mechanism for managing sharing between user processes and the operating system itself.

  In general, **each process has it own private code, data, heap, and stack areas** that are not shared with any other process. In this case, the operating system creates page tables that map the corresponding virual pages to disjoint physical pages. However, in some instances it is desirable for processes the share code and data.

  For example, every process must call the same operating system kernel code, and every C program makes calls to routines in the standard C library such as `printf`. Rather than including separate copies of the kernel and standard C library in each process, the operating system can arrange for multiple processes to share a single copy of the code by **mapping the appropriate virtual pages in different processes to the same physical pages**.

  ![separate_addr_space](./pic/separate_addr_space.png)

  <p align="center">VM provides separate address spaces, the figure from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/17-vm-concepts.pdf">cmu-213 slide</a></p>

+ ***Simplifying memory allocation***: When a program running for allocating additional memory to user processes. When a program running in a user process requests additional heap space, the operating system allocates an appropriate number, say 𝐾, of contiguous virtual memory pages, and **maps them to 𝐾 arbitrary physical pages** located anywhere in physical memory. Because of the way page tables work, there is no need for the operating system to locate 𝐾 contiguous pages of physical memory. **The pages can be scattered randomly in physical memory**. 

#### VM as a Tool for Memory Protection

Providing separate virtual address space makes it easy to isolate the private memories of different processes. But the address translation mechanism can be extended in an natural to provide more finer access control. Since the address translation hardware reads a PTE each time the CPU generates an address, it is straightforward to control access to the content of a virtual page by adding some additional permission bits to PTE.

Specifically:

+ A user process should not be allowed to modify its read-only code setion;
+ A user process should be allowed to read or modify any of the code and data structures in the kernel;
+ A user process should not be allowed to read or write the private memory of other processes;
+ A user process should not be allowed to modify any virtual pages that are shared with other processes, unless all parties explicitly allow it.(via calls to explicit interprocess communication system calls)

![mem_protection](./pic/mem_protection.png)

<p align="center">VM Protect the Memory as a Tool, the figure from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/17-vm-concepts.pdf">cmu-213 slide</a></p>

+ Processes running in ***kernel mode*** can access any page, but processes running in ***user mode*** are only allowed to access the pages fpr which **SUP is 0**.
+ The READ and WRITE bits contorl read and write access to the page.
+ If the instruction violates these permissions, then the CPU triggers a **general protection fault** that transfers control to an exception handler in the kernel, which sens a `SIGSEGV `signal to the offending process. Linux typically report this exception as a `segmentation fault`.

#### Address Translation

Formally, address translation is a  mapping between the elements of an 𝑁-element virtual address space(VAS) and 𝑀-element physical address space(PAS), `MAP: VAS ⟹ PAS U {∅}`, where 𝑁 usually larger than 𝑀

![addr_trans_equation](./pic/addr_trans_equation.png)

<p align="center">Address translation equation, the figure from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/17-vm-concepts.pdf">cmu-213 slide</a></p>



![addr_trans_pg_tb](./pic/addr_trans_pg_tb.png)

<p align="center">How MMU uses the page table to perform the mapping, the figure from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/17-vm-concepts.pdf">cmu-213 slide</a></p>

A control register in the CPU, the ***page table base register(PTBR)*** points to  the current page table. 

The 𝒏-bit virtual  address has two components: 

+ a 𝒑-bit ***virtual page offset(VPO)*** 
+ an (𝒏 - 𝒑)-bit ***virtual page number(VPN)***

The **MMU** uses the **VPN** to select the appropriate **PTE**. 

The corresponding physical address is the concatenation of the **physical page number** from **the page table entry** and the **VPO** from the virtual address.

Notice that since the physical and virtual pages are both 𝑷 bytes, ***the physical page offset(PPO)*** is identical to the ***VPO***.

##### Address Translation: Page Hit

![addr_trans_page_hit](./pic/addr_trans_page_hit.png)

<p align="center">Page hit in the address translation, the figure from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/17-vm-concepts.pdf">cmu-213 slide</a></p>

​    ⓵       Processor sends virtual address to MMU

​    ⓶-⓷ MMU fetches PTE from page table in memory

​    ⓸      MMU sends physical address to cache/memory

​    ⓹      Cache/memory sends word to processor

##### Address Translation: Page fault

![addr_trans_page_fault](./pic/addr_trans_page_fault.png)

<p align="center">Page fault in the address translation, the figure from <a href = "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/17-vm-concepts.pdf">cmu-213 slide</a></p>

​    ⓵       Processor sends virtual address to MMU

​    ⓶-⓷ MMU fetches PTE from page table in memory

​    ⓸       Valid bit is zero, so MMU triggers page fault exception

​    ⓹       Handler identifies victim(and, if dirty, page it out to disk)

​    ⓺       Handlerr pages in new page and updates PTE in memory

​    ⓻       Handler returns to original process, restarting faulting instruction

   #### Speeding Up Address Translation with a TLB(Translation Lookaside Buffer)

Every time the CPU generates a virtual address, the **MMU must refer to a PTE** in order to translate the virtual address into a physical address. **In the worst case**, this requires an addtional fetch from memory, at a cost of tens to hundreds of cycles. However, many systems try to eliminate  evem this cost by including a small cache of PTEs in the MMU called a ***translationi lookaside buffer(TLB)***.

A TLB is a small, virtually addressed cache where each line holds a block consisting of a single PT, where it maps virtual page numbers to physical page numbers. A TLB usually has a high degree of associativity.

In TLB, the ***TLB index(TLBI)*** and ***TLB tag(TLBT)*** that are used for **set selection** and **line matching** are extracted from the virtual page number in the virtual address, where if TLB has `𝑇 = 2ᵗ`, ***TLBL*** consists of the **𝙩 least significant bits** of VPN, and ***TLBT*** consists of **remaining bits** in the VPN.

![tlb_fetch_process](./pic/tlb_fetch_process.jpeg)

<p align="center">TLB fetch process</p>
