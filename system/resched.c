/* resched.c - resched, resched_cntl */

#include <xinu.h>
#include <string.h>

struct	defer	Defer;

void printReadyList(qid16 q);
/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{

	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	//sid: group creation	
	
	int SRcount=getSize(SRreadylist);
	int TScount=getSize(TSreadylist);
	int totalready=SRcount+TScount;

	//sid: print processes
	printReadyList(SRreadylist);
	printReadyList(TSreadylist);

	
	XDEBUG_KPRINTF("\n[Size: %d, SRcount: %d, TScount: %d]\n",totalready, SRcount, TScount);
			
	//sid: changing group priority
	ptold = &proctab[currpid];	

	XDEBUG_KPRINTF("Current Process-> %s, %d\n",ptold->prname,ptold->group);
	
	if (strncmp(ptold->prname, "prnull",6) != 0)
	{	
		SRcount=SRcount-1;      
	}	

	if(ptold->group==SRTIME){
		chgprio(SRTIME,INITIAL_PRIORITY+SRcount);
		chgprio(TSSCHED,getgprio(TSSCHED)+TScount);	
	}
	else if(ptold->group==TSSCHED){
		chgprio(SRTIME,getgprio(SRTIME)+SRcount);
		chgprio(TSSCHED,INITIAL_PRIORITY+TScount);		
	}	


	XDEBUG_KPRINTF("[SRTIME->%d, TSSCHED->%d]\n",getgprio(SRTIME), getgprio(TSSCHED));	

	//select SR group for scheduling
	if(getgprio(SRTIME)>=getgprio(TSSCHED)){
		XDEBUG_KPRINTF("SR group selected\n");


		
		
	}
	else{ //Select TS group for scheduling
		XDEBUG_KPRINTF("TS group selected\n");

	}


	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];

	if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		if (ptold->prprio > firstkey(SRreadylist)) {
			return;
		}

		/* Old process will no longer remain current */

		ptold->prstate = PR_READY;
		insert(currpid, SRreadylist, ptold->prprio);
	}

	/* Force context switch to highest priority ready process */

	currpid = dequeue(SRreadylist);
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		/* Reset time slice for process	*/

	//sid: old vs new info
	//XDEBUG_KPRINTF("[Old %s->%d, New %s->%d]\n",ptold->prname,ptold->group,ptnew->prname,ptnew->group);

	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

	/* Old process returns here when resumed */

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}

void printReadyList(qid16 q){
	int index=0;	
	pid32 tpid=getIthItem(q,index);
	struct procent *tpidEntry;
	while(tpid!=EMPTY){
		tpidEntry = &proctab[tpid];
		XDEBUG_KPRINTF("(%s,%d)->",tpidEntry->prname,tpidEntry->group);		
		index++;
		tpid=getIthItem(q,index);
	
	}

	XDEBUG_KPRINTF("No: %d\n",index);		
}


	// /* Point to process table entry for the current (old) process */

	// ptold = &proctab[currpid];

	// if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
	// 	if (ptold->prprio > firstkey(readylist)) {
	// 		return;
	// 	}

	// 	/* Old process will no longer remain current */

	// 	ptold->prstate = PR_READY;
	// 	insert(currpid, readylist, ptold->prprio);
	// }

	//  Force context switch to highest priority ready process 

	// currpid = dequeue(readylist);
	// ptnew = &proctab[currpid];
	// ptnew->prstate = PR_CURR;
	// preempt = QUANTUM;		/* Reset time slice for process	*/

	// //sid: old vs new info
	// //XDEBUG_KPRINTF("[Old %s->%d, New %s->%d]\n",ptold->prname,ptold->group,ptnew->prname,ptnew->group);

	// ctxsw(&ptold->prstkptr, &ptnew->prstkptr);