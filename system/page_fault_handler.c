#include<xinu.h>

//@sid- page fault handler

void page_fault_handler(void){


	XDEBUG_KPRINTF("Page fault occured\n");

	return;

}