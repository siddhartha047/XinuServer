/* kill.c - kill */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */

int32 restoreframes(pid32 pid);

syscall	kill(
	  pid32		pid		/* ID of process to kill	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
	int32	i;			/* Index into descriptors	*/

	mask = disable();



	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	//kprintf("KILL : %d\n", pid);
	if (--prcount <= 1) {		/* Last user process completes	*/
		xdone();
	}
	
	send(prptr->prparent, pid);
	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}

  // Lab3 TODO. Free frames as a process gets killed.
	//restore frames and stuff

	XDEBUG_KPRINTF("Frame before kill->");
	printFrameList(frame_head);

	if(restoreframes(pid)==SYSERR){
		XDEBUG_KPRINTF("restore frame failed\n");		
	}

	XDEBUG_KPRINTF("Frame after kill->");
	printFrameList(frame_head);
	// //


	freestk(prptr->prstkbase, prptr->prstklen);

	switch (prptr->prstate) {
	case PR_CURR:
		prptr->prstate = PR_FREE;	/* Suicide */
		resched();

	case PR_SLEEP:
	case PR_RECTIM:
		unsleep(pid);
		prptr->prstate = PR_FREE;
		break;

	case PR_WAIT:
		semtab[prptr->prsem].scount++;
		/* Fall through */

	case PR_READY:
		getitem(pid);		/* Remove from queue */
		/* Fall through */

	default:
		prptr->prstate = PR_FREE;
	}

	restore(mask);
	return OK;
}


int32 restoreframes(pid32 pid){
	frame_t *frame_entry;
	inverted_page_t *inverted_page_entry;

	wait(fault_sem);
	
	backing_store_map *bs_map_entry;

	//int32 errorflag=remove_bs_map(pid);
	int32 errorflag=FALSE;
	for(int i=0;i<MAX_BS_ENTRIES;i++){
		bs_map_entry=&backing_store_map_tab[i];
		if(bs_map_entry->pid==pid){
			if(deallocate_bs(i)==SYSERR){
				XDEBUG_KPRINTF("Kill deallocate bs failed\n");
				//panic("kill deallocate failed\n");
				errorflag=TRUE;
				signal(fault_sem);
				return SYSERR;
			}
			bs_map_entry->pid=-1;
			bs_map_entry->vpn=0;
			bs_map_entry->npages=0;
			bs_map_entry->bsid=-1;
			bs_map_entry->allocated=0;
		}
	}


	
	if(USE_HEAP_TO_TRACK==FALSE){
		xmemlist_t	*prev, *curr, *next;
		struct procent *prptr;
		
		prptr=&proctab[pid];
		
		prev=&prptr->prxmemlist;
		curr=prev->mnext;

		while(curr!=NULL){
			next=curr->mnext;
			if(freemem((char*)curr,sizeof(xmemlist_t))==SYSERR){
				//panic("memory tracking nodes cleaning failed\n");
				signal(fault_sem);
				return SYSERR;
				
			}
			curr=next;
		}

	}

	for(int i=0;i<NFRAMES;i++){
		inverted_page_entry= &inverted_page_tab[i];
		
		if(inverted_page_entry->pid==pid){
			if(removeFromFrameList(i)==SYSERR){
				XDEBUG_KPRINTF("Something went wrong while removing frame\n");
				//panic("wrong");
				signal(fault_sem);
				return SYSERR;
			}
			frame_entry=&frame_tab[i];
			frame_entry->id=i;
			frame_entry->state=FRAME_FREE;
			frame_entry->type=FRAME_NONE;
			frame_entry->dirty=0;
			frame_entry->next=(frame_t *)NULL;

			inverted_page_entry->refcount=0;
			inverted_page_entry->pid=-1;
			inverted_page_entry->vpn=0;
		}
	}


	signal(fault_sem);

	if(errorflag==SYSERR){
		return SYSERR;
	}

	return OK;
}