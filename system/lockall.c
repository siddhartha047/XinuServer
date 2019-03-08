/*	lockall.c - lockall	*/
#include <xinu.h>
#include <stdlib.h>
/* Lab 2: Complete this function */

int32 comparator(const void *p, const void *q);


 syscall lockall (int32 type, int32 lpriority, int32 numlocks, ...){ 	
	//your implementation goes here

 	intmask mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
	struct	lockent *lockptr;		

	mask = disable();
	uint32		*a;		/* Points to list of args	*/	

	a = (uint32 *)(&numlocks + 1);	/* Start of args		*/
	uint32		*original;		/* Points to list of args	*/
	uint32		*temp;		/* Points to list of args	*/
	original=a;	


	if(!(type==READ || type==WRITE)){
		restore(mask);
		return SYSERR;	
	}	

	prptr = &proctab[currpid];

	
	// XDEBUG_KPRINTF("Before sorting :");
	// for(int i=0;i<numlocks;i++)	{
	// 	int ldes=*a++;				
	// 	XDEBUG_KPRINTF("%d->",ldes);
	// }

	a=original;	
	qsort(a, numlocks,sizeof(a),comparator); 
	a=original;

	//XDEBUG_KPRINTF("\nAfter sorting :");	
	for(int i=0;i<numlocks;i++)	{
		int ldes=*a++;

		if (isbadlock(ldes)) {
			restore(mask);
			return SYSERR;
		}

		lockptr = &locktab[ldes];
		if (lockptr->lstate == L_FREE) {
			restore(mask);
			return SYSERR;
		}	

		//if already locked by same process
		if(prptr->locks[ldes]==1){
			restore(mask);
			return SYSERR;
		}

		//XDEBUG_KPRINTF("%d->",ldes);				
	}
	//XDEBUG_KPRINTF("\n");

	
	int32 status;
	a=original;

	for(int i=0;i<numlocks;i++)	{
		int ldes=*a++;				
		status=lock(ldes, type,lpriority);
		temp=original;

		if(status==SYSERR || status==DELETED){
			resched_cntl(DEFER_START);
			for(int j=0;j<i;j++){
				int templdes=*temp++;
				releaseall(1,templdes);				
			}
			resched_cntl(DEFER_STOP);

			restore(mask);
			return status;
		}

	}
		
	restore(mask);
	return OK;
}



int32 comparator(const void *p, const void *q)  
{ 
 	int32 l = *(const int32 *)p; 
    int32 r = *(const int32 *)q;  
    return (l - r); 
} 
