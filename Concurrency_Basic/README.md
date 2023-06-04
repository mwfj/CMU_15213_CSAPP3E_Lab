# Concurrency Basic

Normally, we treated concurrency mainly as a mechanism that the operating system kernel used to run multiple applicaiton programs. But concurrency is not limted to the kernel. Application-level concurrency is useful in other way as well:

+ ***Accessing slow I/O devices :*** When an application is waiting for data to arrive from a slow I/O device such as a disk, the kernel keeps the CPU busy by running other processes.
+ ***Interacting with humans :*** People who interact with computers demand the ability to perform multiple tasks at the same time.
+ ***Reducing latency by deferring work :*** For example, a dynamic storage allocator might reduce the latency of individual **free** operations by deferring coalescing to a concurrent "coalescing" flow that **runs at a lower priority**, soaking up spare CPU cycles as they become available.
+ ***Servicing multiple network clients :*** For a real server that might be expected to service hundreds or thoudsands of clients per second, it is not acceptable to allow one slow client to deny service to the others. A better approach is to build a ***concurrent server*** that creates a separate logical flow for each client. This allows the server to service multiple clients concurrently and precludes slow clients from monopolizing the server.
+ ***Computing in parallel on multi-core machines :*** Many modern systems are equipped with multi-core processors that contain multiple CPUs. Applications that are partitioned into concurrent concurrent flows often run faster on multi-core machines than on uniprocessor machines because the flows execute in parallel rather than being interleaved.

Application that use application-level concurrency are known as ***concurrent programs***. Modern operating system provide ***three basic approaches for building concurrent programs***:

+ ***Processes :*** With this approach, each logical control flow is a process that is scheduled and maintained by the kernel. Since processes have separate virtual address space, flows that want to communicated with each other must use some kind of explicit ***interprocess communication(IPC)*** mechanism.
+ ***I/O multiplexing :*** This is form of concurrent programming where applications explicitly schedule their own logical flow in the context of a **single process**. **Logical flows are modeled as state machines** that the main program explicitly transitions from state to state as a result of data arriving on file descriptors. **Since the program is a single process, all flows share the same address space**.
+ ***Threads :*** Threads are logical flows that run in the context of **a single process** and are **scheduled by the kernel**. You can think of threads as a hybrid of other two approaches:
  + scheduled by the kernel like process flows
  + shareing the same virtual address space like I/O multiplexing flows.



## Concurrent Programming with Processes



## Concurrent Programming with I/O multiplexing



## Concurrent Programming with Threads

Like processes, ***threads*** are mechanism that permits an application to perform multiple tasks concurrently. A single process can contain multiple threads. However, each thread is very much like a separate tiny process, except for one difference: **they share the same address space and thus can access the same data.**



The state of a single thread is thus very similar to that of a process. It has a **program counter(PC)** that tracks where the program is fetching instructions from. Each thread has their own private set of registers it uses computation; thus, if there are two threads that are running on a singple processor, when switching from running one(T1) to running the other(T2), a ***context switch*** must take place.