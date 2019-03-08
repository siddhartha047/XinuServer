#include<xinu.h>


void priorityUpdate(int32 pid){	

	struct	lockent *lockptr;
	//if not waiting return
	struct	procent *prptr;
	prptr=&proctab[pid];

	if(prptr->prstate==PR_READY && prptr->lockid==NOT_WAITING){		
		ready(getitem(pid));
		return;
	}

	if(prptr->lockid==NOT_WAITING){
		return;
	}


	int ldes=prptr->lockid;		
	lockptr = &locktab[ldes];	
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

	return;
}