/* getitem.c - getfirst, getlast, getitem */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  getfirst  -  Remove a process from the front of a queue
 *------------------------------------------------------------------------
 */
pid32	getfirst(
	  qid16		q		/* ID of queue from which to	*/
	)				/* Remove a process (assumed	*/
					/*   valid with no check)	*/
{
	pid32	head;

	if (isempty(q)) {
		return EMPTY;
	}

	head = queuehead(q);
	return getitem(queuetab[head].qnext);
}

/*------------------------------------------------------------------------
 *  getlast  -  Remove a process from end of queue
 *------------------------------------------------------------------------
 */
pid32	getlast(
	  qid16		q		/* ID of queue from which to	*/
	)				/* Remove a process (assumed	*/
					/*   valid with no check)	*/
{
	pid32 tail;

	if (isempty(q)) {
		return EMPTY;
	}

	tail = queuetail(q);
	return getitem(queuetab[tail].qprev);
}

/*------------------------------------------------------------------------
 *  getitem  -  Remove a process from an arbitrary point in a queue
 *------------------------------------------------------------------------
 */
pid32	getitem(
	  pid32		pid		/* ID of process to remove	*/
	)
{
	pid32	prev, next;

	next = queuetab[pid].qnext;	/* Following node in list	*/
	prev = queuetab[pid].qprev;	/* Previous node in list	*/
	queuetab[prev].qnext = next;
	queuetab[next].qprev = prev;
	return pid;
}

//sid: get i^th process id from queue


pid32	getIthItem(
	  qid16	q,
	  int index		/* getIth entry from queue*/
	)
{

	pid32	head,tail;

	if (isempty(q)) {
		return EMPTY;
	}

	head = queuehead(q);
	tail = queuetail(q);

	int i=0;
	while(queuetab[head].qnext!=tail && i<index){
		head=queuetab[head].qnext;		
		i++;
	}

	if(queuetab[head].qnext==tail){
		return EMPTY;
		//XDEBUG_KPRINTF("Something went wrong: index exceded queue size\n");
	}

	return queuetab[head].qnext;	
}


//sid: size computation

int	getSize(
	  qid16	q	  
	)
{

	pid32	head,tail;

	if (isempty(q)) {
		return 0;
	}

	head = queuehead(q);
	tail = queuetail(q);

	int i=0;
	while(queuetab[head].qnext!=tail){
		head=queuetab[head].qnext;		
		i++;
	}

	return i;	
}