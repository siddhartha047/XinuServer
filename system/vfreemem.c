/* vfreemem.c - vfreemem */

#include <xinu.h>

syscall	vfreememUsingIdentity(char	*blkaddr, uint32 nbytes);
syscall	vfreememUsingHeap(char	*blkaddr, uint32 nbytes);



syscall	vfreemem(
	  char		*blkaddr,	/* Pointer to memory block	*/
	  uint32	nbytes		/* Size of block in bytes	*/
	)
{
  // Lab3 TODO.

	if(USE_HEAP_TO_TRACK){
		return vfreememUsingHeap(blkaddr,nbytes);
	}
	else{
		return vfreememUsingIdentity(blkaddr, nbytes);
	}
	
	return OK;
}

syscall	vfreememUsingIdentity(
	  char		*blkaddr,	/* Pointer to memory block	*/
	  uint32	nbytes		/* Size of block in bytes	*/
	)
{
  // Lab3 TODO.

	intmask	mask;			/* Saved interrupt mask		*/
	struct	xmemlist	*next, *prev, *block;
	uint32	top;
	struct procent *prptr;

	mask = disable();

	prptr=&proctab[currpid];


	if ((nbytes == 0) || ((uint32) blkaddr < (uint32) (NBPG*VPN0))
			  || ((uint32) blkaddr > (uint32) (prptr->prhsize + VPN0) * NBPG)) {
		restore(mask);
		return SYSERR;
	}

	nbytes = (uint32) roundmb(nbytes);	/* Use memblk multiples	*/
	block = (xmemlist_t *)getmem(sizeof(xmemlist_t));
	block->vheapaddr=blkaddr;
	block->mlength=nbytes;


	prev = &prptr->prxmemlist;			/* Walk along free list	*/
	next = prev->mnext;


	while ((next != NULL) && (next->vheapaddr < block->vheapaddr)) {
		prev = next;
		next = next->mnext;
	}

	if (prev == &prptr->prxmemlist) {		/* Compute top of previous block*/
		top = (uint32) NULL;
	} else {
		top = (uint32) prev->vheapaddr + prev->mlength;
	}

	//XDEBUG_KPRINTF("Top -> %d",top);

	/* Ensure new block does not overlap previous or next blocks	*/

	if (((prev->vheapaddr != prptr->prxmemlist.vheapaddr) && (uint32) block->vheapaddr < top)
	    || ((next != NULL)	&& (uint32) block->vheapaddr+nbytes > (uint32)next->vheapaddr)) {
		restore(mask);
		return SYSERR;
	}

	prptr->prxmemlist.mlength += nbytes;

	/* Either coalesce with previous block or add to free list */
	int32 blockfree=0;
	xmemlist_t *blocktemp=0;

	if (top == (uint32) block->vheapaddr) { 	/* Coalesce with previous block	*/
		prev->mlength += nbytes;
		blocktemp=block;
		block = prev;
		blockfree=1;

	} else {			/* Link into list as new node	*/
		block->mnext = next;
		block->mlength = nbytes;
		prev->mnext = block;
	}

	/* Coalesce with next block if adjacent */

	int32 nextfree=0;
	if (((uint32) block->vheapaddr + block->mlength) == (uint32) next->vheapaddr) {
		block->mlength += next->mlength;
		block->mnext = next->mnext;
		nextfree=1;
	}

	if(blockfree==1)freemem(blocktemp,sizeof(xmemlist_t));
	if(nextfree==1)freemem(next,sizeof(xmemlist_t));
	restore(mask);

	return OK;
}

syscall	vfreememUsingHeap(
	  char		*blkaddr,	/* Pointer to memory block	*/
	  uint32	nbytes		/* Size of block in bytes	*/
	)
{
  // Lab3 TODO.

	intmask	mask;			/* Saved interrupt mask		*/
	struct	memblk	*next, *prev, *block;
	uint32	top;
	struct procent *prptr;

	mask = disable();

	prptr=&proctab[currpid];


	if ((nbytes == 0) || ((uint32) blkaddr < (uint32) (NBPG*VPN0))
			  || ((uint32) blkaddr > (uint32) (prptr->prhsize + VPN0) * NBPG)) {
		restore(mask);
		return SYSERR;
	}

	nbytes = (uint32) roundmb(nbytes);	/* Use memblk multiples	*/
	block = (struct memblk *)blkaddr;

	prev = &prptr->prmemlist;			/* Walk along free list	*/
	next = prev->mnext;
	while ((next != NULL) && (next < block)) {
		prev = next;
		next = next->mnext;
	}

	if (prev == &prptr->prmemlist) {		/* Compute top of previous block*/
		top = (uint32) NULL;
	} else {
		top = (uint32) prev + prev->mlength;
	}

	/* Ensure new block does not overlap previous or next blocks	*/

	if (((prev != &prptr->prmemlist) && (uint32) block < top)
	    || ((next != NULL)	&& (uint32) block+nbytes>(uint32)next)) {
		restore(mask);
		return SYSERR;
	}

	prptr->prmemlist.mlength += nbytes;

	/* Either coalesce with previous block or add to free list */

	if (top == (uint32) block) { 	/* Coalesce with previous block	*/
		prev->mlength += nbytes;
		block = prev;
	} else {			/* Link into list as new node	*/
		block->mnext = next;
		block->mlength = nbytes;
		prev->mnext = block;
	}

	/* Coalesce with next block if adjacent */

	if (((uint32) block + block->mlength) == (uint32) next) {
		block->mlength += next->mlength;
		block->mnext = next->mnext;
	}
	restore(mask);

	return OK;
}

