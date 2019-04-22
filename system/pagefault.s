#include<icu.s>
.text
.globl pagefault
pagefault:
	pushal
	pushfl
	call page_fault_handler
	popfl
	popal
	add $0x4, %esp
	iret
/*
#include<icu.s>
.text
error_code: .long 0
.globl pagefault,error_code
pagefault:
	popl error_code
	call disable
	pushal
	pushfl
	call page_fault_handler
	popfl
	popal
	call restore
	iret
*/
/*
#include<icu.s>
.text
error_code: .long 0
	.globl  pagefault, error_code

pagefault:
    popl error_code 
    pushfl
    cli             
    pushal          
    call page_fault_handler
    popal        
    popfl
iret
*/

/*

#include<icu.s>
.text
.globl pagefault
pagefault:
	pushal
	pushfl
	call page_fault_handler
	popfl
	popal
	add $0x4, %esp
	iret
*/