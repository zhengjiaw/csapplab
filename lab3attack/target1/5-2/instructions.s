0x4019d6:	48 8d 04 37          	lea    (%rdi,%rsi,1),%rax

a2 19 40 00 00 00 00 00  /* 0x4019a2  movq %rax, %rdi 0x4019a0 + 2 */
0x401a06    401a03:	8d 87 41 48 89 e0    mov rsp, %rax


0x4019ea    4019e8:	8d 87 89 ce      mov ecx, esi	
0x401a70    401a6e:	c7 07 89 d1      mov edx, ecx
0x4019dd    4019db:	b8 5c 89 c2      mov eax, edx


ab 19 40 00 00 00 00 00  /* popq rax 0x4019ab 0x4019a7 + 4 */
/* rsp string 偏移为 32 */ 20 00 00 00 00 00 00 00   

	

