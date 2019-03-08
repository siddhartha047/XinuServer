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

	if(!(type==READ || type==WRITE)){
		restore(mask);
		return SYSERR;	
	}

	lockptr = &locktab[ldes];
	if (lockptr->lstate == L_FREE) {
		restore(mask);
		return SYSERR;
	}	

	prptr = &proctab[currpid];

	//if already locked by same process
	if(prptr->locks[ldes]==1){
		restore(mask);
		return SYSERR;
	}
	

	int locktimestamp=lockptr->timestamp;
	lockptr->lmode[currpid]=type;

	//sid: locking logics goes here
	// XDEBUG_KPRINTF("Trying lock %d-> rcount: %d, wcount: %d, rwait: %d, wwait: %d maxprio: %d\n",ldes,lockptr->rcount,lockptr->wcount,lockptr->rwait,lockptr->wwait,lockptr->maxprio);
	// XDEBUG_KPRINTF("Trying Lock Wait Status: ");
	// for(int i=0;i<10;i++){
	// 	if(lockptr->wprocess[i]==LPR_WAIT){
	// 		XDEBUG_KPRINTF("(%d, %d, %d, %d)->",lockptr->wprocess[i],lockptr->lmode[i],(&proctab[i])->prprio,(&proctab[i])->prinh);	
	// 	}		
	// }

	// XDEBUG_KPRINTF("\nTrying Lock Hold Status: ");
	// for(int i=0;i<10;i++){
	// 	if((&proctab[i])->locks[ldes]==1){
	// 		XDEBUG_KPRINTF("(%d, %d, %d)->",lockptr->lmode[i],(&proctab[i])->prprio,(&proctab[i])->prinh);	
	// 	}	
	// }	

	// XDEBUG_KPRINTF("\nTrying Current %d : prio->%d prinh->%d lockid->%d\n",currpid, prptr->prprio,prptr->prinh,prptr->lockid);
	//

	if((lockptr->rcount+lockptr->wcount)==0){
		//no one holding the lock		
		if(type==READ){
			lockptr->rcount++;
		}
		else{
			lockptr->wcount++;
		}

		//lockptr->maxprio=getprioinh(currpid);

	}
	else if(lockptr->rcount>0){
		//readers holding lock
		
		//no writers waiting and lock request READ type 		
		
		if(type==WRITE){			
			lockptr->wwait++;											
			insertlckprio(ldes, currpid, lockptr->lqueue, lpriority, type);						
			resched();		
		}
		//if no writer waiting just reader 
		else if(type==READ && lockptr->wwait==0){
			//gets the lock
			lockptr->rcount++;
			//lockptr->maxprio=max2(lockptr->maxprio,getprioinh(currpid)); //update overall maxprio								
		}		
		//a higher priority writer is waiting
		else if(type==READ && lpriority>=getMaxWriterPriority(lockptr->lqueue, ldes)){
			//gets the lock	
			lockptr->rcount++;	
			//lockptr->maxprio=max2(lockptr->maxprio,getprioinh(currpid)); //update overall maxprio					
		
			//take all reader process before first write if there are any
			resched_cntl(DEFER_START);
			while(lockptr->lmode[lockptr->lqueue]!=WRITE){								
				lockptr->rwait--;
				int pidtemp=dequeue(lockptr->lqueue);
				lockptr->wprocess[pidtemp]=LPR_FREE;
				
				//lockptr->maxprio=max2(lockptr->maxprio,getprioinh(pidtemp)); //update based on overall maxprio					
				lockptr->rcount++;
				ready(pidtemp);				
			}			
			resched_cntl(DEFER_STOP);

		}
		else{
			//a high priority writer is waiting reader goes to sleep
			lockptr->rwait++;									
			insertlckprio(ldes, currpid, lockptr->lqueue, lpriority, type);			
			resched();			
		}

	}
	else if(lockptr->wcount>0){
		//write lock holding so put in waiting state
		if(type==READ)lockptr->rwait++;	
		else lockptr->wwait++;		
				
		insertlckprio(ldes, currpid, lockptr->lqueue, lpriority, type);		
		resched();		
	}
	else{
		kprintf("This shouldn't happen\n");
	}

	//after awake from resched

	if(lockptr->lstate == L_FREE || locktimestamp!=lockptr->timestamp){
		prptr->locks[ldes]=0;
		restore(mask);
		return DELETED;		
	}

	prptr->locks[ldes]=1;
	prptr->lockid=NOT_WAITING;

	//after awaking hold the lock;	

	//sid: locking logics goes here
	// XDEBUG_KPRINTF("Got lock %d-> rcount: %d, wcount: %d, rwait: %d, wwait: %d maxprio: %d\n",ldes,lockptr->rcount,lockptr->wcount,lockptr->rwait,lockptr->wwait,lockptr->maxprio);
	// XDEBUG_KPRINTF("Got Lock Wait Status: ");
	// for(int i=0;i<10;i++){
	// 	if(lockptr->wprocess[i]==LPR_WAIT){
	// 		XDEBUG_KPRINTF("(%d, %d, %d, %d)->",lockptr->wprocess[i],lockptr->lmode[i],(&proctab[i])->prprio,(&proctab[i])->prinh);	
	// 	}		
	// }

	// XDEBUG_KPRINTF("\nGot Lock Hold Status: ");
	// for(int i=0;i<10;i++){
	// 	if((&proctab[i])->locks[ldes]==1){
	// 		XDEBUG_KPRINTF("(%d, %d, %d)->",lockptr->lmode[i],(&proctab[i])->prprio,(&proctab[i])->prinh);	
	// 	}	
	// }	

	// XDEBUG_KPRINTF("\nTrying Current %d : prio->%d prinh->%d lockid->%d\n",currpid, prptr->prprio,prptr->prinh,prptr->lockid);

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