/*	lock.c - lock */
#include <xinu.h>

/* Lab 2: Complete this function */

int getMaxWriterPriority(qid16 q, int32 ldes);

syscall lock(int32 ldes, int32 type, int32 lpriority) {

        //your implementation goes here
	//most of the logic can be implemented here
	//consider different cases as mentioned in the handout
	intmask mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
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

	prptr = &proctab[currpid];
	

	int locktimestamp=lockptr->timestamp;
	lockptr->lmode[currpid]=type;

	//sid: locking logics goes here

	if((lockptr->rcount+lockptr->wcount)==0){
		//no one holding the lock		
		if(type==READ){
			lockptr->rcount++;
		}
		else if(type==WRITE){
			lockptr->wcount++;
		}
		else{
			XDEBUG_KPRINTF("invalid lock type\n");
		}		

	}
	else if(lockptr->rcount>0){
		//readers holding lock
		
		//no writers waiting and lock request READ type 		
		
		if(type==WRITE){			
			lockptr->wwait++;			
			prptr->prstate = PR_WAIT;
			insertlckprio(ldes, currpid, lockptr->lqueue, lpriority, type);						
			resched();
		}
		//if no writer waiting just reader 
		else if(type==READ && lockptr->wwait==0){
			//gets the lock
			lockptr->rcount++;					
		}		
		//a higher priority writer is waiting
		else if(type==READ && lpriority>=getMaxWriterPriority(lockptr->lqueue, ldes)){
			//gets the lock	
			lockptr->rcount++;			
			//take all reader process before first writer			
			resched_cntl(DEFER_START);
			while(lockptr->lmode[lockptr->lqueue]!=WRITE){								
				ready(dequeue(lockptr->lqueue));
			}			
			resched_cntl(DEFER_STOP);

		}
		else{
			//a high priority writer is waiting reader goes to sleep
			lockptr->rwait++;
			lockptr->lmode[currpid]=type;
			prptr->prstate = PR_WAIT;
			insertlckprio(ldes, currpid, lockptr->lqueue, lpriority, type);			
			resched();
		}

	}
	else if(lockptr->wcount>0){
		//write lock holding so put in waiting state
		if(type==READ){
			lockptr->rwait++;
		}
		else{
			lockptr->wwait++;
		}

		lockptr->lmode[currpid]=type;
		prptr->prstate = PR_WAIT;
		insertlckprio(ldes, currpid, lockptr->lqueue, lpriority, type);		
		resched();
	}

	//after awake from resched

	if(lockptr->lstate == L_FREE || locktimestamp!=lockptr->timestamp){
		restore(mask);
		return DELETED;		
	}

	prptr->locks[ldes]=1;

	restore(mask);

	return OK;
}


int getMaxWriterPriority(qid16 q, int32 ldes){
	if (isempty(q)) {
		XDEBUG_KPRINTF("q shouldn't be empty\n");
		return EMPTY;
	}	

	int curr = firstid(q);

	struct	lockent *lockptr;
	lockptr = &locktab[ldes];
	while(lockptr->lmode[curr]!=WRITE){
		curr = queuetab[curr].qnext;
	}

	return queuetab[curr].qkey;
}