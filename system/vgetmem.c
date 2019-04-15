/* vgetmem.c - vgetmem */

#include <xinu.h>

char  	*vgetmem(
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
		struct memblk *vmemblk=(struct memblk*)vpn_to_address(VPN0);
		vmemblk->mlength=(prptr->prhsize*NBPG);
		vmemblk->mnext=NULL;	
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
