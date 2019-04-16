#include<xinu.h>

//frame  management will be done here

int32 initialize_frame(void){

	intmask mask;
	mask=disable();

	frame_head=(frame_t *)NULL;
	
	//sid: set up frames
	for(int i=0;i<NFRAMES;i++){		
		(&frame_tab[i])->id=i;
		(&frame_tab[i])->state=FRAME_FREE;
		(&frame_tab[i])->type=FRAME_NONE;
		(&frame_tab[i])->dirty=FRAME_NOT_DIRTY;
		(&frame_tab[i])->next=(frame_t *)NULL;
	}

	for(int i=0;i<NFRAMES;i++){
		(&inverted_page_tab[i])->pid=-1;
		(&inverted_page_tab[i])->vpn=0;
		(&inverted_page_tab[i])->refcount=0;
	}

	restore(mask);

	return OK;
}

int32 get_one_frame(void){
	intmask mask=disable();
	

	int32 frameNo=find_free_frame();

	if(frameNo==SYSERR){
		XDEBUG_KPRINTF("Shouldn't happen\n");
		restore(mask);
		return SYSERR;
	}

	XDEBUG_KPRINTF("Frame found %d\n",frameNo);

	frame_t *frame_entry;
	frame_entry=&frame_tab[frameNo];
	frame_entry->state=FRAME_USED;
	frame_entry->type=FRAME_NONE;
	frame_entry->dirty=FRAME_NOT_DIRTY;
	frame_entry->next=(frame_t *)NULL;

	inverted_page_t *inverted_page_entry;
	inverted_page_entry=&inverted_page_tab[frameNo];
	inverted_page_entry->refcount=0;

	//for maintaining FIFO policy
	addToFrameList(frame_entry);

	printFrameList(frame_head);

	restore(mask);
	return frameNo;
}

int32 find_free_frame(void){
	intmask mask=disable();
	frame_t *frame_entry;
	int32 frameNo=-1;

	for(int i=0;i<NFRAMES;i++){
		frame_entry=&frame_tab[i];
		if(frame_entry->state==FRAME_FREE){
			frameNo=i;
			restore(mask);
			return frameNo;
		}
	}

	XDEBUG_KPRINTF("doing page replacement\n");		
	//page replacement will be done here later one
	if(currpolicy == FIFO){
		frameNo=get_frame_fifo();
	}
	else if(currpolicy==GCA){
		frameNo=get_frame_gca();
	}
	else{
		panic("invalid policy\n");
	}	


	if(frameNo==SYSERR){
		restore(mask);
		return SYSERR;
	}

	int32 status=swap_frame_back(frameNo);
	
	if(status==SYSERR){
		restore(mask);
		return SYSERR;	
	}

	restore(mask);
	return frameNo;
}



int32 swap_frame_back(int32 frameNo){

	intmask mask=disable();

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

		panic("Something is wrong\n");

		pdptr[pd_offset].pd_pres=0;

		hook_ptable_delete(ptframeNo);
		remove_frame_fifo(ptframeNo);
	}

	//dirty then write back
	if(ptptr[pt_offset].pt_dirty==1|| frame_tab[frameNo].dirty==1){


		backing_store_map *bs_map_entry;
		bs_map_entry=get_bs_map(pid,vpn);

		if(bs_map_entry==NULL){
			XDEBUG_KPRINTF("swapping gon wrong at finding map\n");
			kill(pid);
			restore(mask);
			return SYSERR;
		}

		uint32 offset=vpn-bs_map_entry->vpn;
		bsd_t bsid=bs_map_entry->bsid;

		//opeing
		if(open_bs(bsid)==SYSERR){
			kill(pid);
			restore(mask);
			return SYSERR;
		}

		//writing
		if(write_bs((char *)frameno_to_address(frameNo),bsid,offset)==SYSERR){
			kill(pid);
			restore(mask);
			return SYSERR;	
		}
		
		//closing
		if(close_bs(bsid)==SYSERR){
			kill(pid);
			restore(mask);
			return SYSERR;		
		}

	}
	//hook out
	hook_pswap_out(pid,vpn,frameNo);
	restore(mask);
	return OK;
}