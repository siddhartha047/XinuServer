#include<xinu.h>

//@author memory management related to page will be handled here
pt_t *global_pt[GLOBAL_PAGE_NO];
pt_t *device_pt; //global page directory
pd_t *global_pd; //global page directory


pt_t *get_one_page(void);
pd_t* get_page_directory(pid32 pid);

//sid: 4 page and 1 device memory
int32 initialize_global_page_table(void){
	// intmask mask=disable();
	pt_t *pt_entry;	
	pt_t *pt_device_entry;


	for(int i=0;i<GLOBAL_PAGE_NO;i++){
		if((pt_entry=get_one_page())==NULL){
			XDEBUG_KPRINTF("Couldn't allocate global page\n");
			// restore(mask);
			return SYSERR;
		}

		for(int j=0;j<PAGETABSIZE;j++){
			pt_entry[j].pt_pres=1;
			pt_entry[j].pt_base=i*PAGETABSIZE+j;
		}

		global_pt[i]=pt_entry;
	}
	
	if((pt_device_entry=get_one_page())==NULL){
		XDEBUG_KPRINTF("Couldn't allocate device page\n");
		// restore(mask);
		return SYSERR;
	}

	for(int j=0;j<PAGETABSIZE;j++){
		pt_device_entry[j].pt_pres=1;
		pt_device_entry[j].pt_base=DEVICE_VPN+j;
	}

	device_pt=pt_device_entry;
	// restore(mask);


	return OK;
}

pt_t *get_one_page(void){
	// intmask mask=disable();	
	
	int32 frameNo=get_one_frame();

	if(frameNo==SYSERR){
		XDEBUG_KPRINTF("Couldn't find a frame for the page\n");
		// restore(mask);
		return (pt_t *)NULL;
	}

	frame_t *frame_entry;
	frame_entry=&frame_tab[frameNo];
	frame_entry->type=FRAME_PT;

	pt_t *pt_entry;
	pt_entry=(pt_t *)frameno_to_address(frameNo);
	
	for(int i=0;i<PAGETABSIZE;i++){
		//pt_entry[i]=(pt_t){0,1,0,0,0,0,0,0,0,0,0};
		pt_entry[i].pt_pres  = 0;		/* page is present?		*/
	    pt_entry[i].pt_write = 1;		/* page is writable?		*/
		pt_entry[i].pt_user	= 0;		/* is use level protection?	*/
		pt_entry[i].pt_pwt	= 0;		/* write through for this page? */
		pt_entry[i].pt_pcd	= 0;		/* cache disable for this page? */
		pt_entry[i].pt_acc	= 0;		/* page was accessed?		*/
		pt_entry[i].pt_dirty = 0;		/* page was written?		*/
		pt_entry[i].pt_mbz	= 0;		/* must be zero			*/
		pt_entry[i].pt_global= 0;		/* should be zero in 586	*/
		pt_entry[i].pt_avail = 0;		/* for programmer's use		*/
		pt_entry[i].pt_base = 0;         // this will be aloocated when page is created
	}

	hook_ptable_create(frameNo);


	// restore(mask);
	return pt_entry;
}

pd_t* get_page_directory(pid32 pid){
	// intmask mask=disable();

	int32 frameNo=get_one_frame();

	if(frameNo==SYSERR){
		// restore(mask);
		return (pd_t*)NULL;
	}

	inverted_page_tab[frameNo].pid=pid;

	frame_t *frame_entry;
	frame_entry=&frame_tab[frameNo];
	frame_entry->type=FRAME_PD;

	pd_t *pd_entry;
	pd_entry=(pd_t*)frameno_to_address(frameNo);

	
	for(int i=0;i<PAGEDIRSIZE;i++){
		//pd_entry[i]=(pd_t){0,1,0,0,0,0,0,0,0,0,0};
		pd_entry[i].pd_pres	= 0;		/* page table present?		*/
		pd_entry[i].pd_write = 1;		/* page is writable?		*/
		pd_entry[i].pd_user	= 0;		/* is use level protection?	*/
		pd_entry[i].pd_pwt	= 0;		/* write through cachine for pt? */
		pd_entry[i].pd_pcd	= 0;		/* cache disable for this pt?	*/
		pd_entry[i].pd_acc	= 0;		/* page table was accessed?	*/
		pd_entry[i].pd_mbz	= 0;		/* must be zero			*/
		pd_entry[i].pd_fmb	= 0;		/* four MB pages?		*/
		pd_entry[i].pd_global= 0;
		pd_entry[i].pd_avail = 0;
		pd_entry[i].pd_base  = 0;

	}

	for(int i=0;i<GLOBAL_PAGE_NO;i++){
		pd_entry[i].pd_pres=1;
		pd_entry[i].pd_write=1;
		pd_entry[i].pd_base=address_to_vpn(global_pt[i]);
	}

	pd_entry[DEVICE_PTN].pd_pres=1;
	pd_entry[DEVICE_PTN].pd_write=1;
	pd_entry[DEVICE_PTN].pd_base=address_to_vpn(device_pt);


	// restore(mask);
	return pd_entry;
}

