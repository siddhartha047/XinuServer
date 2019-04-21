#include<xinu.h>


int32 get_frame_gca(void){

	//intmask mask=disable();
	frame_t *frame_entry;
	inverted_page_t *inverted_page_entry;
	struct procent *prptr;

	int32 frameNo=(lframeNo+1)%NFRAMES;

	uint32 vpn;
	pid32 pid;
	vd_t *vaddptr;
	pd_t *pdptr;
	pt_t *ptptr;
	uint32 pd_offset;
	uint32 pt_offset;
	//uint32 pg_offset;
	uint32 vaddress;

	for(int i=0;i<=SWEEP_TIMES*NFRAMES;i++){
		frame_entry=&frame_tab[frameNo];

		//only replace resident pages
		if(frame_entry->type==FRAME_PR){
			
			inverted_page_entry=&inverted_page_tab[frameNo];

			pid=inverted_page_entry->pid;
			vpn=inverted_page_entry->vpn;
			prptr=&proctab[pid];

			vaddress=vpn_to_address(vpn);
			vaddptr=(vd_t *)&vaddress;
			pd_offset=vaddptr->pd_offset;
			pt_offset=vaddptr->pt_offset;
			//pg_offset=vaddptr->pg_offset;


			pdptr=prptr->prpd;
			ptptr=(pt_t *)vpn_to_address(pdptr[pd_offset].pd_base);
			ptptr=ptptr+pt_offset;

			//updating clock bits
			//0,0 // 1,0 // 1,1
			if(ptptr->pt_acc==0 && ptptr->pt_dirty==0){
				//remove this one
				lframeNo=frameNo;
				removeFromFrameList(frameNo);
				//restore(mask);
				return frameNo;
			}
			else if(ptptr->pt_acc==1 && ptptr->pt_dirty==0){
				ptptr->pt_acc=0;
			}
			else if(ptptr->pt_acc==1 && ptptr->pt_dirty==1){
				ptptr->pt_dirty=0;
				frame_entry->dirty=1;
			}			

		}

		frameNo=(frameNo+1)%NFRAMES;
	}

	XDEBUG_KPRINTF("Shouldn't happen for gca\n");
	panic("gca gone wrong\n");

	//restore(mask);
	return SYSERR;
}