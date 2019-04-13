#include<icu.s>
.text
.globl pagefault
pagefault:
	pushal
	pushfl
	call page_fault_handler
	popfl
	popal
	add $0x4, %esp /*skip 4 bytes */
	iret