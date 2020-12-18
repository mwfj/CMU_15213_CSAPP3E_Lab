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

## Phase_3
When we see the assemble code in the phase 3, we can see there has one indirect jump and six direct jumps, we can guess that there must be a switch structure in the code inside.
Also, in the assembly code, not every jump will trigger the bomb. Thus, we can guess that, only default case will trigger the bomb, where **trigger every case except default one will avoid this bomb**.

```asm
(gdb) disas phase_3Dump of assembler code for function phase_3:   0x0000000000400f43 <+0>:	sub    $0x18,%rsp   0x0000000000400f47 <+4>:	lea    0xc(%rsp),%rcx   0x0000000000400f4c <+9>:	lea    0x8(%rsp),%rdx   0x0000000000400f51 <+14>:	mov    $0x4025cf,%esi # the arguement that pass in to sscanf   0x0000000000400f56 <+19>:	mov    $0x0,%eax   0x0000000000400f5b <+24>:	callq  0x400bf0 <__isoc99_sscanf@plt> #  call sscanf   0x0000000000400f60 <+29>:	cmp    $0x1,%eax # if the input case less or equal than one, trigger the bomb   0x0000000000400f63 <+32>:	jg     0x400f6a <phase_3+39>   0x0000000000400f65 <+34>:	callq  0x40143a <explode_bomb>   0x0000000000400f6a <+39>:	cmpl   $0x7,0x8(%rsp) # if the first input number higher or equal then 7, trigger the bomb   0x0000000000400f6f <+44>:	ja     0x400fad <phase_3+106>   0x0000000000400f71 <+46>:	mov    0x8(%rsp),%eax # clear the eax register   0x0000000000400f75 <+50>:	jmpq   *0x402470(,%rax,8)   0x0000000000400f7c <+57>:	mov    $0xcf,%eax # case 0 comparsion value   0x0000000000400f81 <+62>:	jmp    0x400fbe <phase_3+123>   0x0000000000400f83 <+64>:	mov    $0x2c3,%eax # case 2   0x0000000000400f88 <+69>:	jmp    0x400fbe <phase_3+123>    0x0000000000400f8a <+71>:	mov    $0x100,%eax # case 3   0x0000000000400f8f <+76>:	jmp    0x400fbe <phase_3+123>   0x0000000000400f91 <+78>:	mov    $0x185,%eax # case 4   0x0000000000400f96 <+83>:	jmp    0x400fbe <phase_3+123>   0x0000000000400f98 <+85>:	mov    $0xce,%eax# case 5   0x0000000000400f9d <+90>:	jmp    0x400fbe <phase_3+123>   0x0000000000400f9f <+92>:	mov    $0x2aa,%eax # case 6   0x0000000000400fa4 <+97>:	jmp    0x400fbe <phase_3+123>   0x0000000000400fa6 <+99>:	mov    $0x147,%eax   0x0000000000400fab <+104>:	jmp    0x400fbe <phase_3+123> # should be the default case   0x0000000000400fad <+106>:	callq  0x40143a <explode_bomb>   0x0000000000400fb2 <+111>:	mov    $0x0,%eax   0x0000000000400fb7 <+116>:	jmp    0x400fbe <phase_3+123>   0x0000000000400fb9 <+118>:	mov    $0x137,%eax # case 1   0x0000000000400fbe <+123>:	cmp    0xc(%rsp),%eax # compare with the number that equal to the number given in each case   0x0000000000400fc2 <+127>:	je     0x400fc9 <phase_3+134>   0x0000000000400fc4 <+129>:	callq  0x40143a <explode_bomb> # if not, trigger the bomb   0x0000000000400fc9 <+134>:	add    $0x18,%rsp   0x0000000000400fcd <+138>:	retq   End of assembler dump.
```
Then, we need to guess the number of input and the type of input. To solve this, we can check the arguement that pass into sscanf, which should be nearest resigester the reveiced the variable(`0x0000000000400f51 <+14>: mov    $0x4025cf,%esi`).

When we print the varible `$0x4025cf` by string, we can see that it is `0x4025cf: "%d %d"`, and thus we know that two digits should be input to the program in this phase. Furthermore, there has another way to guess the number of input, where it is `0x0000000000400f60 <+29>: cmp    $0x1,%eax`, we can know that the number of input should higher than one. 

```bash
(gdb) x/s 0x4025cf0x4025cf:	"%d %d"
(gdb) i r eax # when run the address at 0x0000000000400f60eax            0x2	2```

As the analysis above, we can guess that the **first arguement should be the index to trigger each case(from 0 to 6) and the other arguement is used to do the comparison with key word** in each case.

Then what we need to do is to **check the first indirect jump and calculate the keyword in each case** as the comment show on the assemble code.

```bash
(gdb) p/x *(0x402470+8) # when index = 1 : 311$7 = 0x400fb9(gdb) p/d 0x137$8 = 311(gdb) p/x *(0x402470+16)  # when index = 2 : 707$9 = 0x400f83(gdb) p/d 0x2c3$10 = 707(gdb) p/x *(0x402470+24)  # when index = 3 : 256$11 = 0x400f8a(gdb) p/d 0x100$12 = 256(gdb) p/x *(0x402470+32)  # when index = 4 : 389$13 = 0x400f91(gdb) p/d 0x185$14 = 389(gdb) p/x *(0x402470+40)  # when index = 5 : 206$15 = 0x400f98(gdb) p/d 0xce$16 = 206(gdb) p/x *(0x402470+48)  # when index = 6 : 682$17 = 0x400f9f(gdb) p/d 0x2aa$18 = 682
(gdb) p/x *0x402470 # when index = 0 : 207$19 = 0x400f7c(gdb) p/d 0xcf$20 = 207```

From now, we have already calculate each target address of each case and its relative keyword:

+ 0 207
+ 1 311
+ 2 707 
+ 3 256
+ 4 389
+ 5 206
+ 6 682

### Choose any one of the answers can avoid to trigger the bomb.