/* setuid.c - set uid of a process*/

#include <xinu.h>

/*------------------------------------------------------------------------
 *  chgprio  -  Change the userof a process
 *------------------------------------------------------------------------
 */

int32 setuid(
	  int	newuid
	)
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();
	
	ptold = &proctab[currpid];

	if(ptold->uid==ROOT_USER){
		ptold->uid=newuid;		
	}
	else{
		restore(mask);
		return  SYSERR;	
	}

	restore(mask);
	return OK;
}
