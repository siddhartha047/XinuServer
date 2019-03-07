/*	lock.c - lock */
#include <xinu.h>

/* Lab 2: Complete this function */

int getMaxInheritedPriority(qid16 q){
	if (isempty(q)) {
		XDEBUG_KPRINTF("q shouldn't be empty\n");
		return EMPTY;
	}	

	int curr = firstid(q);
	int tail= queuetail(q);
	
	int maxprio=getprioinh(curr);

	curr = queuetab[curr].qnext;

	while(curr!=tail){
		int currprio=getprioinh(curr);
		if(currprio>maxprio){
			maxprio=currprio;
		}
		curr = queuetab[curr].qnext;
	}

	return maxprio;
}