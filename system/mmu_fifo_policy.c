#include<xinu.h>


int32 get_frame_fifo(void){

	// intmask mask=disable();
	int32 frameNo;

	frame_t *curr;
	curr=frame_head;

	while(curr!=NULL){
		if(curr->type==FRAME_PR){
			frameNo=curr->id;
			if(removeFromFrameList(frameNo)==SYSERR){
				panic("Frame removing from list failed\n");
			}
			// restore(mask);
			return frameNo;
		}
		curr=curr->next;
	}

	panic("I think you have minimum no of frame\n");

	// restore(mask);
	return SYSERR;
}

int32 removeFromFrameList(int32 frameNo){
	// intmask mask=disable();
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
			// restore(mask);
			return OK;
		}
		prev=curr;
		curr=curr->next;		
	}

	// restore(mask);
	return SYSERR;
}


void addToFrameList(frame_t *frame_entry){
	
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

}

void printFrameList(frame_t *frame_entry){
	uint32 vpn;
	vd_t *vaddptr;
	uint32 vaddress;
	uint32 pd_offset;
	uint32 pt_offset;
	pt_t *ptptr;
	pd_t *pdptr;

	XDEBUG_KPRINTF("FIFO :");
	while(frame_entry!=NULL){
		vpn=inverted_page_tab[frame_entry->id].vpn;
		vaddress=vpn_to_address(vpn);
		vaddptr=(vd_t *)&vaddress;
		pd_offset=vaddptr->pd_offset;
		pt_offset=vaddptr->pt_offset;
		//pg_offset=vaddptr->pg_offset;


		pdptr=proctab[inverted_page_tab[frame_entry->id].pid].prpd;
		ptptr=(pt_t *)vpn_to_address(pdptr[pd_offset].pd_base);
		ptptr=ptptr+pt_offset;

			
		XDEBUG_KPRINTF("->(%d, %d, %d, %d)",frame_entry->id, frame_entry->dirty,ptptr->pt_acc,ptptr->pt_dirty);
		frame_entry=frame_entry->next;
	}
	XDEBUG_KPRINTF("\n");
}