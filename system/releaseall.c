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
		//XDEBUG_KPRINTF("%d-> ",ldes);
		
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

		if(lockptr->lmode[currpid]==READ){
			lockptr->rcount--;			
		}
		else{
			lockptr->wcount--;
		}

		if((lockptr->rwait+lockptr->wwait)>0){

			if(lockptr->lmode[firstid(lockptr->lqueue)]==READ && lockptr->wwait>0){				
				//remove all reader before first writer
				while(lockptr->lmode[firstid(lockptr->lqueue)]==READ){
					lockptr->rwait--;
					ready(dequeue(lockptr->lqueue));
				}

			}
			else if(lockptr->lmode[firstid(lockptr->lqueue)]==READ && lockptr->wwait==0){
				//remove all reader
				while(!isempty(lockptr->lqueue)){
					lockptr->rwait--;
					ready(dequeue(lockptr->lqueue));
				}				
			}
			else{				
				//remove the writer
				lockptr->wwait--;
				ready(dequeue(lockptr->lqueue));
			}

		}

		prptr->locks[ldes]=0;

	}	

	resched_cntl(DEFER_STOP);

	restore(mask);

	return OK;
}
