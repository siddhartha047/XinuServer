/* getprioinh.c - getprioinh */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  getprioinh  -  Return the scheduling priority of a process
 *------------------------------------------------------------------------
 */
syscall	getprioinh(
	  pid32		pid		/* Process ID			*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	uint32	prio;			/* Priority to return		*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}
	prio = max2(proctab[pid].prprio,proctab[pid].prinh);
	restore(mask);
	return prio;
}
