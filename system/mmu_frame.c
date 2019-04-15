#include<xinu.h>

//frame  management will be done here
int32 initialize_frame(void);
int32 get_one_frame(void);
int32 find_free_frame(void);
void printFrameList(frame_t *frame_entry);
int32 remove_frame_fifo(int32 frameNo);



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
	if(frame_head==NULL){
		frame_head=frame_entry;
	}
	else{
		frame_t *temp_frame_entry;
		temp_frame_entry=frame_head;

		while(temp_frame_entry->next!=NULL){
			temp_frame_entry=temp_frame_entry->next;
		}

		temp_frame_entry->next=frame_entry;

	}

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

	XDEBUG_KPRINTF("Shouldn't reach here now\n");		
	//page replacement will be done here later one
	

	restore(mask);
	return SYSERR;
}

void printFrameList(frame_t *frame_entry){
	XDEBUG_KPRINTF("FIFO :");
	while(frame_entry!=NULL){
		XDEBUG_KPRINTF("->%d",frame_entry->id);
		frame_entry=frame_entry->next;
	}
	XDEBUG_KPRINTF("\n");
}


int32 remove_frame_fifo(int32 frameNo){
	intmask mask=disable();
	frame_t *prev;
	frame_t *curr;

	curr=frame_head;
	prev=NULL;

	while(curr!=NULL){
		if(curr->id==frameNo){
			//if this is head
			if(prev==NULL){
				frame_head=curr->next;				
			}
			else{
				prev->next=curr->next;				
			}
			curr->next=(frame_t *)NULL;
			curr->state=FRAME_FREE;
			curr->type=FRAME_NONE;
			restore(mask);
			return OK;
		}
		prev=curr;
		curr=curr->next;		
	}

	restore(mask);
	return SYSERR;
}