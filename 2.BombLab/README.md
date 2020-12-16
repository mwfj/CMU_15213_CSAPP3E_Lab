# Bomb Lab Report:

In this lab, we are required

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
### We get the key string to avoid bomb

```bash
(gdb) x/s 0x4024000x402400:	"Border relations with Canada have never been better."
```