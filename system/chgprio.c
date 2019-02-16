/* chgprio.c - chgprio */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  chgprio  -  Change the scheduling priority of group
 *------------------------------------------------------------------------
 */

pri16	chgprio(
	  int		group,		/* group no to change	*/
	  pri16		newprio		/* New priority			*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();
	pri16	oldprio;		/* Priority to return		*/


	if(group==SRTIME){
		oldprio = SR_INITIAL_PRIORITY;
		SR_INITIAL_PRIORITY = newprio;	
	}
	else if(group==TSSCHED){
		oldprio=TS_INITIAL_PRIORITY;
		TS_INITIAL_PRIORITY=newprio;
	}
	else{
		restore(mask);
		return (pri16) SYSERR;
	}
	
	restore(mask);
	return oldprio;
}
