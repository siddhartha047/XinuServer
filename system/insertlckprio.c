/* insertlckprio.c - insert by lock priority */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  insert  -  Insert a process into a queue in descending key order and read
 *------------------------------------------------------------------------
 */
status	insertlckprio(
	  int32 ldes,   //lock descriptor
	  pid32		pid,		/* ID of process to insert	*/
	  qid16		q,		/* ID of queue to use		*/
	  int32		key,		/* Key for the inserted process	*/
	  byte type //locktype
	)
{
	int16	curr;			/* Runs through items in a queue*/
	int16	prev;			/* Holds previous node index	*/

	if (isbadqid(q) || isbadpid(pid)) {
		return SYSERR;
	}
	if(isbadlock(ldes)){
		return SYSERR;
	}

	curr = firstid(q);

	struct	lockent *lockptr;
	lockptr = &locktab[ldes];

	while (queuetab[curr].qkey > key || 
		(queuetab[curr].qkey == key && type==READ)||
		(queuetab[curr].qkey == key && type==lockptr->lmode[curr])) {
		curr = queuetab[curr].qnext;
	}

	struct	procent *prptr;
	prptr=&proctab[pid];

	prptr->prstate = PR_WAIT;
	prptr->lockid= ldes;	
	lockptr->wprocess[pid]=LPR_WAIT;	

	/* Insert process between curr node and previous node */

	prev = queuetab[curr].qprev;	/* Get index of previous node	*/
	queuetab[pid].qnext = curr;
	queuetab[pid].qprev = prev;
	queuetab[pid].qkey = key;
	queuetab[prev].qnext = pid;
	queuetab[curr].qprev = pid;
	return OK;
}
