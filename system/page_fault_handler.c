#include<xinu.h>

//@sid- page fault handler

void page_fault_handler(void){
	intmask mask=disable();
	unsigned long cr2=read_cr2();

	XDEBUG_KPRINTF("[Page fault occured-> (%s, %d)]\n",proctab[currpid].prname,currpid);

	vd_t *faultaddr;
	pd_t *pd_entry;
	pt_t *pt_entry;
	struct procent *prptr;
	uint32 vpn;
	backing_store_map *bs_map_entry;

	wait(fault_sem);
	
	fault_counts++;

	prptr=&proctab[currpid];
	pd_entry=prptr->prpd;

	//get the faulty page address
	
	vpn=address_to_vpn(cr2);	
	faultaddr=(vd_t *)(&cr2);

	
	//check if valid

	uint32 pd_offset=faultaddr->pd_offset;
	uint32 pt_offset=faultaddr->pt_offset;
	//uint32 pg_offset=faultaddr->pg_offset;

	pd_entry+=pd_offset;
	//poitns to correct page table
	if(pd_entry->pd_pres==0){
		pt_entry=get_one_page();
		if(pt_entry==NULL){
			XDEBUG_KPRINTF("No page available\n");		
			kill(currpid);
			signal(fault_sem);
			restore(mask);
			return;
		}
		pd_entry->pd_pres=1;
		pd_entry->pd_base=address_to_vpn(pt_entry);
	}
	else{
		pt_entry=(pt_t*)vpn_to_address(pd_entry->pd_base);
	}



	if((bs_map_entry=get_bs_map(currpid,vpn))==NULL){
		XDEBUG_KPRINTF("Invalid address from pagefault\n");		
		kill(currpid);
		signal(fault_sem);
		restore(mask);
		return;
	}

	//get page table infor
	uint32 pg_offset= vpn-bs_map_entry->vpn;
	int32 frameNo=address_to_frameno(pt_entry);
	inverted_page_tab[frameNo].refcount++;

	//XDEBUG_KPRINTF("Fault: Page table frame %d refcount %d\n",frameNo,inverted_page_tab[frameNo].refcount);

	int32 newframeNo=get_one_frame();
	
	if(newframeNo==SYSERR){
		XDEBUG_KPRINTF("Not enough free frame\n");		
		kill(currpid);
		signal(fault_sem);
		restore(mask);
		return;
	}

	frame_tab[newframeNo].type=FRAME_PR;
	inverted_page_tab[newframeNo].vpn=vpn;	
	inverted_page_tab[newframeNo].pid=currpid;


	if(open_bs(bs_map_entry->bsid)==SYSERR){
		XDEBUG_KPRINTF("Opening backing store failed\n");		
		kill(currpid);
		signal(fault_sem);
		restore(mask);
		return;	
	}

	if(read_bs((char *)frameno_to_address(newframeNo),bs_map_entry->bsid,pg_offset)==SYSERR){
		XDEBUG_KPRINTF("Frame loading from backing store failed\n");		
		kill(currpid);
		signal(fault_sem);
		restore(mask);
		return;	
	}

	if(close_bs(bs_map_entry->bsid)==SYSERR){
		XDEBUG_KPRINTF("Closing backing store failed\n");		
		kill(currpid);
		signal(fault_sem);
		restore(mask);
		return;	
	}
	

	pt_entry[pt_offset].pt_pres=1;
	pt_entry[pt_offset].pt_base=frameno_to_vpn(newframeNo);

	frame_md.curframe=newframeNo;
	hook_pfault(currpid, (void *)cr2, vpn, newframeNo);

	XDEBUG_KPRINTF("<--[Page fault handled (%s, %d)]\n",proctab[currpid].prname,currpid);
	

	
	signal(fault_sem);

	restore(mask);

	return;

}