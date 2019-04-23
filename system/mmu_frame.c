#include<xinu.h>

//frame  management will be done here

int32 initialize_frame(void){

	// intmask mask;
	// mask=disable();

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

	// restore(mask);

	return OK;
}

int32 get_one_frame(void){
	// intmask mask=disable();
	
	int32 frameNo;

	if((frameNo=find_free_frame())==SYSERR){
		XDEBUG_KPRINTF("Shouldn't happen\n");
		//restore(mask);
		return SYSERR;
	}

	//XDEBUG_KPRINTF("Frame found %d\n",frameNo);

	frame_t *frame_entry;
	frame_entry=&frame_tab[frameNo];
	
	frame_entry->state=FRAME_USED;
	frame_entry->type=FRAME_NONE;
	frame_entry->dirty=FRAME_NOT_DIRTY;
	frame_entry->next=(frame_t *)NULL;

	inverted_page_t *inverted_page_entry;
	inverted_page_entry=&inverted_page_tab[frameNo];

	inverted_page_tab[frameNo].pid=currpid;

	inverted_page_entry->refcount=0;

	//for maintaining FIFO policy
	addToFrameList(frame_entry);

	//printFrameList(frame_head);

	// restore(mask);
	return frameNo;
}

int32 find_free_frame(void){
	// intmask mask=disable();
	int32 frameNo=-1;

	for(int i=0;i<NFRAMES;i++){
		if(frame_tab[i].state==FRAME_FREE){
			frameNo=i;
			// restore(mask);
			return frameNo;
		}
	}

	//XDEBUG_KPRINTF("doing page replacement\n");		
	//page replacement will be done here later one
	if(currpolicy==GCA){
		if((frameNo=get_frame_gca())==SYSERR){
			// restore(mask);
			return SYSERR;
		}
	}
	else if(currpolicy == FIFO){
		if((frameNo=get_frame_fifo())==SYSERR){
			// restore(mask);
			return SYSERR;
		}
	}
	else{
		panic("invalid policy\n");
	}

	frame_tab[frameNo].state=FRAME_USED; //added by me
	
	int32 status=swap_frame_back(frameNo);
	
	if(status==SYSERR){
		// restore(mask);
		return SYSERR;	
	}

	// restore(mask);
	return frameNo;
}

