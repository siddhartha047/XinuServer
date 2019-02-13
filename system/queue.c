/* queue.c - enqueue, dequeue */

#include <xinu.h>

struct qentry	queuetab[NQENT];	/* Table of process queues	*/

/*------------------------------------------------------------------------
 *  enqueue  -  Insert a process at the tail of a queue
 *------------------------------------------------------------------------
 */
pid32	enqueue(
	  pid32		pid,		/* ID of process to insert	*/
	  qid16		q		/* ID of queue to use		*/
	)
{
	qid16	tail, prev;		/* Tail & previous node indexes	*/

	if (isbadqid(q) || isbadpid(pid)) {
		return SYSERR;
	}

	tail = queuetail(q);
	prev = queuetab[tail].qprev;

	queuetab[pid].qnext  = tail;	/* Insert just before tail node	*/
	queuetab[pid].qprev  = prev;
	queuetab[prev].qnext = pid;
	queuetab[tail].qprev = pid;
	return pid;
}

/*------------------------------------------------------------------------
 *  dequeue  -  Remove and return the first process on a list
 *------------------------------------------------------------------------
 */
pid32	dequeue(
	  qid16		q		/* ID of queue to use		*/
	)
{
	pid32	pid;			/* ID of process removed	*/

	if (isbadqid(q)) {
		return SYSERR;
	} else if (isempty(q)) {
		return EMPTY;
	}

	pid = getfirst(q);
	queuetab[pid].qprev = EMPTY;
	queuetab[pid].qnext = EMPTY;
	return pid;
}


pid32	dequeueMinBurst(
	  qid16		q		/* ID of queue to use		*/
	)
{
	pid32	pid;			/* ID of process removed	*/

	if (isbadqid(q)) {
		return SYSERR;
	} else if (isempty(q)) {
		return EMPTY;
	}

	//find min burst
	pid32	head,tail;	
	head = queuehead(q);
	tail = queuetail(q);

	
	head=queuetab[head].qnext;
	
	struct procent *tpidEntry;			
	tpidEntry = &proctab[head];

	int minBurst=tpidEntry->B;
	pid32 minProcess=head;
	
	while(queuetab[head].qnext!=tail){		
		head=queuetab[head].qnext;	
		tpidEntry = &proctab[head];	
		if(tpidEntry->B<minBurst){ //less will round robin as inserted last
			minBurst=tpidEntry->B;
			minProcess=head;
		}		
	}	
	pid=getitem(minProcess);	
	queuetab[pid].qprev = EMPTY;
	queuetab[pid].qnext = EMPTY;
	
	return pid;
}
