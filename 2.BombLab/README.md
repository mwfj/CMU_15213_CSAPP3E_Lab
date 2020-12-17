# Bomb Lab Report:

In this lab, we are required to defuse the bomb hide in the program. We need to use the disassemble way to find the bombs.

As the hint from the writeup, I firstly use gdb tool to print out the assemble code and figure out the position of bomb. Then use gdb tool to print the 'suspicious value'
to find out the key word to avoid the bomb

## Phase 1:
Bascally, this function is to compare input string with a keyword string, if the string is the keyword, do nothing. 

Explode the bomb otherwise.

**Thus, what we do is to find the key word and use it as the input string to aviod the bomb**

In this assemble code, we can see that `400ee4 <+4>: mov    $0x402400,%esi` is the one of the arguement that pass into the string compare function.

That must be the keyword, because the other arguement is the input string. Therefore, we just need to print the value the of 0x402400 to find out the key word string.

``` asm
(gdb) disas phase_1
Dump of assembler code for function phase_1:
   0x0000000000400ee0 <+0>:	sub    $0x8,%rsp  # push the space for stack (decrease the stack pointer)
   0x0000000000400ee4 <+4>:	mov    $0x402400,%esi  # pass the value to the temperate variable
   0x0000000000400ee9 <+9>:	callq  0x401338 <strings_not_equal> # call the string compare function
   0x0000000000400eee <+14>:	test   %eax,%eax # get the return value from strings_not_equal function
   0x0000000000400ef0 <+16>:	je     0x400ef7 <phase_1+23> # if the return value is one(same string), go to 0x400ef7
   0x0000000000400ef2 <+18>:	callq  0x40143a <explode_bomb>  # if the string is not same, explode the bomb
   0x0000000000400ef7 <+23>:	add    $0x8,%rsp # pop the space for stack (increase the stack pointer)
   0x0000000000400efb <+27>:	retq   #  return the value
End of assembler dump.
```

### The follow assembling code is from the function of `strings_not_equal`

Basically, this function firstly compare the string length between two strings(one in the %rbx, the other in the %rbp);
If the length is not equal, return false and function end.

Otherwise, compare character by character in a loop; if all the same, return true(1); return false(0) otherwise

```asm
(gdb) disas 0x401338Dump of assembler code for function strings_not_equal:   0x0000000000401338 <+0>:	push   %r12 # push the current callee tempare variable   0x000000000040133a <+2>:	push   %rbp # %r12, %rbp, %rbx to the stack   0x000000000040133b <+3>:	push   %rbx # for keeping the old value   0x000000000040133c <+4>:	mov    %rdi,%rbx # record the pass variable 1 to the %rbx   0x000000000040133f <+7>:	mov    %rsi,%rbp # record the pass variable 2 to the %rbx   0x0000000000401342 <+10>:	callq  0x40131b <string_length> # call string_length function to get the string length(I guess)   0x0000000000401347 <+15>:	mov    %eax,%r12d # record the return value in the %r12: string 1   0x000000000040134a <+18>:	mov    %rbp,%rdi   0x000000000040134d <+21>:	callq  0x40131b <string_length> # string 2   0x0000000000401352 <+26>:	mov    $0x1,%edx # %edx = 0x1   0x0000000000401357 <+31>:	cmp    %eax,%r12d # Compare the length with string 1 and string 2   0x000000000040135a <+34>:	jne    0x40139b <strings_not_equal+99> # length not equal, return   0x000000000040135c <+36>:	movzbl (%rbx),%eax   0x000000000040135f <+39>:	test   %al,%al # a loop to compare string character by character   0x0000000000401361 <+41>:	je     0x401388 <strings_not_equal+80> # if condition   0x0000000000401363 <+43>:	cmp    0x0(%rbp),%al # check whether %al is zero   0x0000000000401366 <+46>:	je     0x401372 <strings_not_equal+58> # yes jump to 0x401372   0x0000000000401368 <+48>:	jmp    0x40138f <strings_not_equal+87> # jump to 0x40138f other wise   0x000000000040136a <+50>:	cmp    0x0(%rbp),%al # Loop begin   0x000000000040136d <+53>:	nopl   (%rax)   0x0000000000401370 <+56>:	jne    0x401396 <strings_not_equal+94>   0x0000000000401372 <+58>:	add    $0x1,%rbx # variable 1++   0x0000000000401376 <+62>:	add    $0x1,%rbp # variable 2++   0x000000000040137a <+66>:	movzbl (%rbx),%eax   0x000000000040137d <+69>:	test   %al,%al   0x000000000040137f <+71>:	jne    0x40136a <strings_not_equal+50> # Back to loop if not equal   0x0000000000401381 <+73>:	mov    $0x0,%edx   0x0000000000401386 <+78>:	jmp    0x40139b <strings_not_equal+99>   0x0000000000401388 <+80>:	mov    $0x0,%edx   0x000000000040138d <+85>:	jmp    0x40139b <strings_not_equal+99>   0x000000000040138f <+87>:	mov    $0x1,%edx   0x0000000000401394 <+92>:	jmp    0x40139b <strings_not_equal+99>   0x0000000000401396 <+94>:	mov    $0x1,%edx   0x000000000040139b <+99>:	mov    %edx,%eax   0x000000000040139d <+101>:	pop    %rbx # get the % rbx   0x000000000040139e <+102>:	pop    %rbp   0x000000000040139f <+103>:	pop    %r12   0x00000000004013a1 <+105>:	retq   # returnEnd of assembler dump.
```
### We get the key string and use it as the input string to avoid bomb

```bash
(gdb) x/s 0x4024000x402400:	"Border relations with Canada have never been better."
```

## Phase_2 :

Next, we move on the `Phase_2`: just like what we did in the phase 1, we first set the break point to the function of phase_2 and disassemble it.

```asm
(gdb) disas phase_2Dump of assembler code for function phase_2:   0x0000000000400efc <+0>:	push   %rbp   0x0000000000400efd <+1>:	push   %rbx   0x0000000000400efe <+2>:	sub    $0x28,%rsp   0x0000000000400f02 <+6>:	mov    %rsp,%rsi   0x0000000000400f05 <+9>:	callq  0x40145c <read_six_numbers> # we guess the input number is six digits numbers   0x0000000000400f0a <+14>:	cmpl   $0x1,(%rsp) # compare *%rsp with 1   0x0000000000400f0e <+18>:	je     0x400f30 <phase_2+52>   0x0000000000400f10 <+20>:	callq  0x40143a <explode_bomb> # trigger the bomb if not equal with one   0x0000000000400f15 <+25>:	jmp    0x400f30 <phase_2+52>   0x0000000000400f17 <+27>:	mov    -0x4(%rbx),%eax # start a loop   0x0000000000400f1a <+30>:	add    %eax,%eax # eax *=2   0x0000000000400f1c <+32>:	cmp    %eax,(%rbx)   0x0000000000400f1e <+34>:	je     0x400f25 <phase_2+41>   0x0000000000400f20 <+36>:	callq  0x40143a <explode_bomb>   0x0000000000400f25 <+41>:	add    $0x4,%rbx # move to next input digit   0x0000000000400f29 <+45>:	cmp    %rbp,%rbx # check whether is jump out of the loop   0x0000000000400f2c <+48>:	jne    0x400f17 <phase_2+27>   0x0000000000400f2e <+50>:	jmp    0x400f3c <phase_2+64>   0x0000000000400f30 <+52>:	lea    0x4(%rsp),%rbx # move to the next input digit and save it to %rbx   0x0000000000400f35 <+57>:	lea    0x18(%rsp),%rbp   0x0000000000400f3a <+62>:	jmp    0x400f17 <phase_2+27> # back to loop begin   0x0000000000400f3c <+64>:	add    $0x28,%rsp   0x0000000000400f40 <+68>:	pop    %rbx   0x0000000000400f41 <+69>:	pop    %rbp   0x0000000000400f42 <+70>:	retq   End of assembler dump.```

As we can see in the assemble code, it call a function called `read_six_numbers`(in 0x400f05), and thus we guess we need to input six numbers for this phase. Let's assume `1 2 3 4 5 6` and see what is going on.

```bash
(gdb) r solution.txtStarting program: /home/mwfj/cmu-15-213-CSAPP3E-lab/2.BombLab/bomb/bomb solution.txtWelcome to my fiendish little bomb. You have 6 phases withwhich to blow yourself up. Have a nice day!Phase 1 defused. How about the next one?1 2 3 4 5 6Breakpoint 1, 0x0000000000400efc in phase_2 ()
```
After calling the `read_six number`, I found that the return number is six, thus I guess this function is to make sure the number digit we input is 6 numbers, where is to check the input validation.

```bash
(gdb) i r raxrax            0x6	6```

Then, we find it compare with 1, if it is not equal with one, trigger the bomb. If so, jump to 0x400f30. Because the first input is 1, thus we jump to 0x400f30.
After jump to 0x400f30, we find a interesting instruction `0x0000000000400f1a <+30>: add    %eax,%eax`, where before running this instruction, we find that the value of eax is 1.

```bash
(gdb) i r eaxeax            0x1	1```

After `0x0000000000400f1a <+30>: add    %eax,%eax` eax double itself compared with rbx, the new input digits. If they are same, skip the bomb, trigger the bomb otherwise.
Thus, our next input is 2, we are temperate safe. Then, the thing we need to to is repeat the stop above and check eax and compare it with *rbx every time

```bash
(gdb) i rrax            0x4	4rbx            0x7fffffffde28	140737488346664rcx            0x0	0rdx            0x7fffffffde34	140737488346676rsi            0x0	0rdi            0x7fffffffd790	140737488344976rbp            0x7fffffffde38	0x7fffffffde38rsp            0x7fffffffde20	0x7fffffffde20r8             0x0	0r9             0x0	0r10            0x7ffff7b80c40	140737349422144r11            0x4025d4	4203988r12            0x400c90	4197520r13            0x7fffffffdf40	140737488346944r14            0x0	0r15            0x0	0rip            0x400f1c	0x400f1c <phase_2+32>eflags         0x202	[ IF ]cs             0x33	51ss             0x2b	43ds             0x0	0es             0x0	0fs             0x0	0gs             0x0	0(gdb) x/d $rbx0x7fffffffde28:	3```

As the gdb show to us, the third input(%rbx) is 3, but the compared value (eax) is 4(2*2), thus it will trigger the bomb. However, we have already discover the input rules, where **the input has six numbers at total, begin as 1, and the next digits is the double as before.**

### Thus the final answer is **1 2 4 8 16 32**

```bash
(gdb) r solution.txt Starting program: /home/mwfj/cmu-15-213-CSAPP3E-lab/2.BombLab/bomb/bomb solution.txtWelcome to my fiendish little bomb. You have 6 phases withwhich to blow yourself up. Have a nice day!Phase 1 defused. How about the next one?1 2 4 8 16 32That's number 2.  Keep going!```