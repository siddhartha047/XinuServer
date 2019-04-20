/* vgetmem.c - vgetmem */

#include <xinu.h>

char  	*vgetmemUsingIdentiy(uint32	nbytes);
char  	*vgetmemUsingHeap(uint32	nbytes);

char  	*vgetmem(
	  uint32	nbytes		/* Size of memory requested	*/
	)
{
  	// Lab3 TODO.
	if(USE_HEAP_TO_TRACK){
		return vgetmemUsingHeap(nbytes);
	}
	else{
		return vgetmemUsingIdentiy(nbytes);
	}

	return (char *)SYSERR;
}

char  	*vgetmemUsingIdentiy(
	  uint32	nbytes		/* Size of memory requested	*/
	)
{
  // Lab3 TODO.
	xmemlist_t	*prev, *curr, *leftover;
	struct procent *prptr;
	
	intmask mask=disable();
	prptr=&proctab[currpid];

	if(nbytes<=0 || prptr->prtype==PR_NORMAL){		
		restore(mask);
		return (char *)SYSERR;
	}


	//this is where process memlist points to now
	//sid: move this to vcreate later one


	nbytes=(uint32)roundmb(nbytes);

	prev=&prptr->prxmemlist;
	curr=prev->mnext;

	
	if(prptr->prmemlistinit==1){
		prptr->prmemlistinit=0;
		
		curr->vheapaddr=vpn_to_address(VPN0);
		curr->mlength=(prptr->prhsize*NBPG);
		curr->mnext=NULL;	
	}

	//taken from getmem
	while (curr != NULL) {			/* Search free list	*/

		if (curr->mlength == nbytes) {	/* Block is exact match	*/
			prev->mnext = curr->mnext;
			prptr->prxmemlist.mlength -= nbytes;
			uint32 vaddress=curr->vheapaddr;
			freemem((char *)curr,sizeof(xmemlist_t));
			restore(mask);
			return (char *)(vaddress);

		} else if (curr->mlength > nbytes) { /* Split big block	*/

			leftover = (xmemlist_t*)getmem(sizeof(xmemlist_t));

			leftover->vheapaddr=((uint32) curr->vheapaddr + nbytes);
			
			prev->mnext = leftover;

			leftover->mnext = curr->mnext;
			leftover->mlength = curr->mlength - nbytes;
			prptr->prxmemlist.mlength -= nbytes;			
			uint32 vaddress=curr->vheapaddr;
			freemem((char *)curr,sizeof(xmemlist_t));			
			restore(mask);
			return (char *)(vaddress);

		} else {			/* Move to next block	*/
			prev = curr;
			curr = curr->mnext;
		}
	}
	restore(mask);
	return (char *)SYSERR;
}



char  	*vgetmemUsingHeap(
	  uint32	nbytes		/* Size of memory requested	*/
	)
{
  // Lab3 TODO.
	struct	memblk	*prev, *curr, *leftover;
	struct procent *prptr;
	
	intmask mask=disable();
	prptr=&proctab[currpid];

	if(nbytes<=0 || prptr->prtype==PR_NORMAL){		
		restore(mask);
		return (char *)SYSERR;
	}


	//this is where process memlist points to now
	//sid: move this to vcreate later one
	if(prptr->prmemlistinit==1){
		prptr->prmemlistinit=0;
		XDEBUG_KPRINTF("A first time page fault here vgetmem----->\n");
		struct memblk *vmemblk=(struct memblk*)vpn_to_address(VPN0);
		vmemblk->mlength=(prptr->prhsize*NBPG);
		vmemblk->mnext=NULL;	
		XDEBUG_KPRINTF("<-----A page fault here fir time vgetmem\n");
	}

	nbytes=(uint32)roundmb(nbytes);

	prev=&prptr->prmemlist;
	curr=prev->mnext;

	//taken from getmem
	while (curr != NULL) {			/* Search free list	*/

		if (curr->mlength == nbytes) {	/* Block is exact match	*/
			prev->mnext = curr->mnext;
			prptr->prmemlist.mlength -= nbytes;
			restore(mask);
			return (char *)(curr);

		} else if (curr->mlength > nbytes) { /* Split big block	*/
			leftover = (struct memblk *)((uint32) curr +
					nbytes);
			prev->mnext = leftover;
			leftover->mnext = curr->mnext;
			leftover->mlength = curr->mlength - nbytes;
			prptr->prmemlist.mlength -= nbytes;
			restore(mask);
			return (char *)(curr);
		} else {			/* Move to next block	*/
			prev = curr;
			curr = curr->mnext;
		}
	}
	
	restore(mask);
	return (char *)SYSERR;
}

void printXMemlist(pid32 pid){

	intmask mask=disable();
	struct procent *prptr;
	prptr=&proctab[pid];
		
	if(USE_HEAP_TO_TRACK){
		struct memblk *curr, *prev;
		prev=&prptr->prmemlist;
		curr=prev->mnext;
		XDEBUG_KPRINTF("Heap Base %d : ",prev->mlength);
		while(curr !=NULL){
			XDEBUG_KPRINTF("(%d,%d)->",curr,curr->mlength);
			curr=curr->mnext;
		}	
		XDEBUG_KPRINTF("\n");	
	}
	else{
		xmemlist_t *curr, *prev;
		prev=&prptr->prxmemlist;
		curr=prev->mnext;
		XDEBUG_KPRINTF("Xinu Base %d : ",prev->mlength);
		while(curr !=NULL){
			XDEBUG_KPRINTF("(%d,%d)->",curr->vheapaddr,curr->mlength);
			curr=curr->mnext;
		}	
		XDEBUG_KPRINTF("\n");	
	}
	
	restore(mask);
}
