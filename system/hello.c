/* hello.c - write */
//Author: @sid

#include <xinu.h>

/*------------------------------------------------------------------------
 *  write  -  Write one or more bytes to a device
 *------------------------------------------------------------------------
 */
syscall	hello(void)
{
	
	XTEST_KPRINTF("Hello system call invoked\n");
	
	int32 retval=0;
	return retval;
}
