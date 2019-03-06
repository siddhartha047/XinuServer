/*	releaseall.c - releaseall	*/
#include <xinu.h>

/* Lab 2: Complete this function */

syscall releaseall (int32 numlocks, ...) {

	//your implementation goes here

	uint32		*a;		/* Points to list of args	*/

	intmask	mask;			/* Saved interrupt mask		*/
	struct	lockent	*lockptr;	/* Ptr to sempahore table entry */

	struct	procent *prptr;		/* Ptr to process' table entry	*/	
	prptr = &proctab[currpid];
	

	mask = disable();

	resched_cntl(DEFER_START);

	a = (uint32 *)(&numlocks + 1);	/* Start of args		*/
	for(int i=0;i<numlocks;i++)	{
		int ldes=*a++;		
		
		if (isbadlock(ldes)) {
			resched_cntl(DEFER_STOP);
		 	restore(mask);
		 	return SYSERR;
		}
		
		lockptr=&locktab[ldes];

		if(lockptr->lstate== L_FREE){
			resched_cntl(DEFER_STOP);
			restore(mask);
		 	return SYSERR;	
		}
		if(prptr->locks[ldes]!=1){
			resched_cntl(DEFER_STOP);
			restore(mask);
		 	return SYSERR;	
		}
		
		XDEBUG_KPRINTF("Trying Release lock %d-> rcount: %d, wcount: %d, rwait: %d, wwait: %d\n",ldes,lockptr->rcount,lockptr->wcount,lockptr->rwait,lockptr->wwait);
		XDEBUG_KPRINTF("Trying Release Lock Status: ");
		for(int i=0;i<10;i++){
			XDEBUG_KPRINTF("(%d, %d)->",lockptr->wprocess[i],lockptr->lmode[i]);
		}	
		XDEBUG_KPRINTF("\nTrying Release Process %d Status: ",currpid);
		for(int i=0;i<10;i++){
			XDEBUG_KPRINTF("%d->",prptr->locks[i]);
		}
		XDEBUG_KPRINTF(": prio->%d prinh->%d lockid->%d\n",prptr->prprio,prptr->prinh,prptr->lockid);


		if(lockptr->lmode[currpid]==READ){
			lockptr->rcount--;			
		}
		else{
			lockptr->wcount--;
		}

		if((lockptr->rwait+lockptr->wwait)>0){

			if(lockptr->lmode[firstid(lockptr->lqueue)]==READ && lockptr->wwait>0){				
				//remove all reader before first writer
				while(lockptr->lmode[firstid(lockptr->lqueue)]!=WRITE){
					lockptr->rwait--;
					int pidtemp=dequeue(lockptr->lqueue);
					lockptr->wprocess[pidtemp]=LPR_FREE;
					ready(pidtemp);
				}

			}
			else if(lockptr->lmode[firstid(lockptr->lqueue)]==READ && lockptr->wwait==0){
				//remove all reader
				while(!isempty(lockptr->lqueue)){
					lockptr->rwait--;
					int pidtemp=dequeue(lockptr->lqueue);
					lockptr->wprocess[pidtemp]=LPR_FREE;
					ready(pidtemp);
				}				
			}
			else{				
				//remove the writer
				lockptr->wwait--;
				int pidtemp=dequeue(lockptr->lqueue);
				lockptr->wprocess[pidtemp]=LPR_FREE;
				ready(pidtemp);
			}

		}

		prptr->locks[ldes]=0;

		
		XDEBUG_KPRINTF("Release lock %d-> rcount: %d, wcount: %d, rwait: %d, wwait: %d\n",ldes,lockptr->rcount,lockptr->wcount,lockptr->rwait,lockptr->wwait);
		XDEBUG_KPRINTF("Release Lock Status: ");
		for(int i=0;i<10;i++){
			XDEBUG_KPRINTF("(%d, %d)->",lockptr->wprocess[i],lockptr->lmode[i]);
		}	
		XDEBUG_KPRINTF("\nRelease Process %d Status: ",currpid);
		for(int i=0;i<10;i++){
			XDEBUG_KPRINTF("%d->",prptr->locks[i]);
		}
		XDEBUG_KPRINTF(": prio->%d prinh->%d lockid->%d\n",prptr->prprio,prptr->prinh,prptr->lockid);

	}	

	resched_cntl(DEFER_STOP);

	restore(mask);

	return OK;
}
