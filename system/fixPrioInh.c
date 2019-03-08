/* fixPrioInh.c - fixing after killing */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  insert  -  Insert a process into a queue in descending key order and read
 *------------------------------------------------------------------------
 */

void recursiveFixInh(int32 pid);


status	fixPrioInh(	  
	  pid32		pid
	)
{
	
	
	struct	lockent *lockptr;

	struct	procent *prptr;
	prptr=&proctab[pid];	
	
	//release all holded locks
	resched_cntl(DEFER_START);
	for(int i=0;i<NLOCKS;i++){
		if(prptr->locks[i]==1){
			releaseall(1,i);
		}
	}
	resched_cntl(DEFER_STOP);


	// XDEBUG_KPRINTF("Killing: %d\r\nBefore fixing inh: ",pid);
	// for(int i=3;i<10;i++){
	// 	XDEBUG_KPRINTF("(%d, %d, %d)->",i,(&proctab[i])->prprio,(&proctab[i])->prinh);
	// }
	// XDEBUG_KPRINTF("\n");
	
	resched_cntl("DEFER_START");

	//if dying in while waiting
	if(prptr->lockid!=NOT_WAITING){
		int ldes=prptr->lockid;		
		lockptr = &locktab[ldes];
		getitem(pid); //remove that from the queue

		int maxprio=0;
		//getmaxpriority from wait queue
		if(nonempty(lockptr->lqueue)){
			maxprio=getMaxInheritedPriority(lockptr->lqueue);
		}

		for(int i=0;i<NPROC;i++){
			// process that are holding that lock the current process was waiting for
			if((&proctab[i])->locks[ldes]==1){
				(&proctab[i])->prinh=maxprio;
				priorityUpdate(i);
			}
		}
		
	}	

	resched_cntl("DEFER_STOP");
	
	// XDEBUG_KPRINTF("After fixing inh :");
	// for(int i=3;i<10;i++){
	// 	XDEBUG_KPRINTF("(%d, %d, %d)->",i,(&proctab[i])->prprio,(&proctab[i])->prinh);
	// }
	// XDEBUG_KPRINTF("\n");
	
	
	return OK;
}


