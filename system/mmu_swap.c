#include<xinu.h>



int32 swap_frame_back(int32 frameNo){

	// intmask mask=disable();

	struct procent *prptr;
	inverted_page_t *inverted_page_entry;
	pd_t *pdptr;
	pt_t *ptptr;

	inverted_page_entry=&inverted_page_tab[frameNo];

	int32 vpn=inverted_page_entry->vpn;
	int32 pid=inverted_page_entry->pid;


	
	int32 vaddress=vpn_to_address(vpn);
	vd_t *vaddressptr=(vd_t *)(&vaddress);
	
	uint32 pd_offset=vaddressptr->pd_offset;
	uint32 pt_offset=vaddressptr->pt_offset;

	prptr=&proctab[pid];
	pdptr=prptr->prpd;

	ptptr=(pt_t *)vpn_to_address(pdptr[pd_offset].pd_base);
	ptptr[pt_offset].pt_pres=0;

	//XDEBUG_KPRINTF("Frame %d, Vpn %d base %d\n",frameNo,vpn,ptptr[pt_offset].pt_base);

	if(pid==currpid){
		tmp=vpn;
		asm("pushl %eax");
		asm("invlpg tmp");
		asm("popl %eax");
	}

	int32 ptframeNo;	
	inverted_page_t *ptinverted_page_entry;

	ptframeNo=pdptr[pd_offset].pd_base-FRAME0;
	ptinverted_page_entry=&inverted_page_tab[ptframeNo];

	ptinverted_page_entry->refcount--;

	//if frame done then remove

	//XDEBUG_KPRINTF("Fault: Page frame %d refcount %d\n",frameNo,ptinverted_page_entry->refcount);

	if(ptinverted_page_entry->refcount==0){

		//panic("Something is wrong\n");
		//XDEBUG_KPRINTF("Removing page table\n");

		pdptr[pd_offset].pd_pres=0;

		removeFromFrameList(ptframeNo);
		hook_ptable_delete(ptframeNo);
		
	}

	//dirty then write back
	if(ptptr[pt_offset].pt_dirty==1|| frame_tab[frameNo].dirty==1){

		XDEBUG_KPRINTF("Dirty Frame: %d  started writing ->%d,%d\n",frameNo,pid,currpid);

		backing_store_map *bs_map_entry;
		bs_map_entry=get_bs_map(pid,vpn);

		if(bs_map_entry==NULL){
			XDEBUG_KPRINTF("swapping gon wrong at finding map\n");
			//signal(fault_sem);
			//kill(pid);
			// restore(mask);
			return SYSERR;
		}

		uint32 offset=vpn-bs_map_entry->vpn;
		bsd_t bsid=bs_map_entry->bsid;

		//opeing
		if(open_bs(bsid)==SYSERR){
			//kill(pid);
			// restore(mask);
			return SYSERR;
		}

		//writing
		if(write_bs((char *)frameno_to_address(frameNo),bsid,offset)==SYSERR){
			//kill(pid);
			// restore(mask);
			return SYSERR;	
		}
		
		//closing
		if(close_bs(bsid)==SYSERR){
			//kill(pid);
			// restore(mask);
			return SYSERR;		
		}

		XDEBUG_KPRINTF("Dirty Frame: %d  finished writing-> %d,%d\n",frameNo,pid,currpid);

	}
	ptptr[pt_offset].pt_pres=0;
	ptptr[pt_offset].pt_acc=0;
	ptptr[pt_offset].pt_dirty=0;
	frame_tab[frameNo].dirty=0;

	// XDEBUG_KPRINTF("Frame %d, Vpn %d base %d\n",frameNo,vpn,ptptr[pt_offset].pt_base);
	// XDEBUG_KPRINTF("Fm no, %d, base: %d\n",frameNo, ptptr[pt_offset].pt_base);
	//hook out
	frame_md.reclaimframe=frameNo;
	hook_pswap_out(pid,vpn,frameNo);
	// restore(mask);
	return OK;
}