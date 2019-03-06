/*	ldelete.c - ldelete 	*/
#include <xinu.h>

/* Lab 2: Complete this function */

/*------------------------------------------------------------------------
 * semdelete  -  Delete a lock by releasing its table entry
 *------------------------------------------------------------------------
 */


syscall ldelete( 
		int32 ldes	/* lock descriptor */
	)
{
	// your implementation
	intmask mask;			/* Saved interrupt mask		*/
	struct	lockent *lockptr;		

	mask = disable();
	if (isbadlock(ldes)) {		
		restore(mask);
		return SYSERR;
	}
	
	lockptr = &locktab[ldes];
	if (lockptr->lstate == L_FREE) {		
		restore(mask);
		return SYSERR;
	}

	lockptr->lstate = L_FREE;
	lockptr->timestamp=clktime;
	int pid=0;
	resched_cntl(DEFER_START);
	while (lockptr->wwait+lockptr->rwait > 0) {	/* Free all waiting processes	*/		
		pid=getfirst(lockptr->lqueue);
		if(lockptr->lmode[pid]==READ){
			lockptr->rwait--;
		}
		else{
			lockptr->wwait--;
		}
		ready(pid);
	}
	resched_cntl(DEFER_STOP);
	restore(mask);	

	return OK;
}
