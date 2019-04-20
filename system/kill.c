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
	//


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

	for(int i=0;i<NFRAMES;i++){
		inverted_page_entry= &inverted_page_tab[i];
		
		if(inverted_page_entry->pid==pid){
			if(removeFromFrameList(i)==SYSERR){
				XDEBUG_KPRINTF("Something went wrong while removing frame\n");
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

	backing_store_map *bs_map_entry;

	wait(fault_sem);
	int32 errorflag=FALSE;
	for(int i=0;i<MAX_BS_ENTRIES;i++){
		bs_map_entry=&backing_store_map_tab[i];
		if(bs_map_entry->pid==pid){
			if(deallocate_bs(i)){
				XDEBUG_KPRINTF("Kill deallocate bs failed\n");
				errorflag=TRUE;
				//return SYSERR;
			}
			bs_map_entry->pid=-1;
			bs_map_entry->vpn=0;
			bs_map_entry->npages=0;
			bs_map_entry->bsid=-1;
			bs_map_entry->allocated=0;
		}
	}

	signal(fault_sem);


	if(errorflag)return SYSERR;

	return OK;
}