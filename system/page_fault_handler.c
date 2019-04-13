#include<xinu.h>

//@sid- page fault handler

void page_fault_handler(void){

	XDEBUG_KPRINTF("Page fault occured\n");

	unsigned long cr2=read_cr2();
	int32 new_frameNo=0;
	uint32 vpn=address_to_vpn(cr2);

	hook_pfault(currpid, (void *)cr2, vpn, new_frameNo);

	return;

}