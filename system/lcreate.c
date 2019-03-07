/*	lcreate.c - lcreate	*/
#include <xinu.h>

/* Lab 2: Complete this function */

local	int	newlock(void);

/*------------------------------------------------------------------------
 *  lockcreate  -  Create a new lock and return the ID to the caller
 *------------------------------------------------------------------------
 */


int lcreate() {

  // your implementation
	intmask	mask;			/* Saved interrupt mask		*/
	int	lockNo;			/* Lock ID to return	*/

	mask = disable();

	if ((lockNo=newlock())==SYSERR) {
		restore(mask);
		return SYSERR;
	}	

	locktab[lockNo].rcount = 0; //no reader holding lock
	locktab[lockNo].wcount = 0; //no writer holding lock
	
	locktab[lockNo].rwait=0; //how many readers waiting
	locktab[lockNo].wwait=0; //how many writers waiting
	locktab[lockNo].maxprio=0;

	locktab[lockNo].timestamp=clktime;

	for(int i=0;i<NPROC;i++){
		locktab[lockNo].lmode[i]=-1;
		locktab[lockNo].wprocess[i]=LPR_FREE;
	}

	restore(mask);	

  	return lockNo;
}


/*------------------------------------------------------------------------
 *  newlock  -  Allocate an unused lock and return its index
 *------------------------------------------------------------------------
 */
local	int	newlock(void)
{
	static	int	nextlock = 0;
	int	sem;			
	int	i;			

	for (i=0 ; i<NLOCKS ; i++) {
		sem = nextlock++;
		if (nextlock >= NLOCKS)
			nextlock = 0;
		if (locktab[sem].lstate == L_FREE) {
			locktab[sem].lstate = L_USED;
			return sem;
		}
	}
	return SYSERR;
}

