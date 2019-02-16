/* getcprio.c - getcprio */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  getcprio  -  Return the scheduling priority of a group
 *------------------------------------------------------------------------
 */

syscall	getgprio(
	  int		group		/* group no			*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	uint32	prio;			/* Priority to return		*/

	mask = disable();

	if(group==SRTIME){
		prio = SR_INITIAL_PRIORITY;		
	}
	else if(group==TSSCHED){
		prio=TS_INITIAL_PRIORITY;		
	}
	else{
		restore(mask);
		return (pri16) SYSERR;
	}
	
	restore(mask);
	return prio;	
}
