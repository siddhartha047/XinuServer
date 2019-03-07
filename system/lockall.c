/*	releaseall.c - releaseall	*/
#include <xinu.h>

/* Lab 2: Complete this function */

 syscall lockall (int32 type, int32 lpriority, int32 numlocks, ...){ 	
	//your implementation goes here

 	intmask mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
	struct	lockent *lockptr;		

	mask = disable();
	uint32		*a;		/* Points to list of args	*/	
	a = (uint32 *)(&numlocks + 1);	/* Start of args		*/
	
	for(int i=0;i<numlocks;i++)	{
		int ldes=*a++;		
		
		if (isbadlock(ldes)) {
			resched_cntl(DEFER_STOP);
		 	restore(mask);
		 	return SYSERR;
		}
	}
		
	restore(mask);

	return OK;
}
