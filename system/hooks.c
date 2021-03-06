/* hooks.c - hook_ptable_create, hook_ptable_delete, hook_pfault, hook_pswap_out */

#include <xinu.h>

#define HOOK_LOG_ON

#define POLICY_TESTING 1

/*
 * Note that this file will be replaced in grading. So you may change them to
 * output what you are interested, but do not let your implementation depend on
 * this file.
 */

/*---------------------------------------------------------------------------
 *  hook_ptable_create  -  Called when your implementation is creating a page
 *  table.
 *---------------------------------------------------------------------------
 */
void hook_ptable_create(uint32 pagenum) {
#ifdef HOOK_LOG_ON
	kprintf("\n=== Created page table %d ===\n", pagenum);
#endif
}

/*---------------------------------------------------------------------------
 *  hook_ptable_delete  -  Called when your implementation is deleting a page
 *  table.
 *---------------------------------------------------------------------------
 */
void hook_ptable_delete(uint32 pagenum) {
#ifdef HOOK_LOG_ON
	kprintf("\n=== Deleted page table %d ===\n", pagenum);
#endif
}

/*---------------------------------------------------------------------------
 *  hook_pfault  -  Called whenever the page fault handler is called.
 *---------------------------------------------------------------------------
 */
void hook_pfault(int16 procid, void *addr, uint32 pagenum, uint32 framenum) {
#ifdef HOOK_LOG_ON
	kprintf("\n=== Page fault for address 0x%0X, %d ===\n", addr, addr);

  if (isbadpid(currpid)) {
    kprintf("[ERR] bad pid\n");
    panic("[FAIL]\n");
  }

#if POLICY_TESTING == 1
  //sid: 
  //frame_md.curframe=framenum;
  // Add to the end of the list (ADIL)
  int16 cf = frame_md.curframe;
  frame_md.alloc_page_proc[cf] = procid;
  frame_md.alloc_page[cf] = pagenum;
  frame_md.curframe++;
  if (frame_md.curframe == NFRAMES) frame_md.curframe = 0;
  kprintf("Proname: %s, Procid: %d, Framenum: %d, Pagenum: %d\n", proctab[procid].prname, procid, framenum, pagenum);
#endif

#endif
}

int32 lastfm=-1;

/*---------------------------------------------------------------------------
 *  hook_pswap_out  -  Called when your implementation is replacing (swapping
 *  out) a frame.
 *---------------------------------------------------------------------------
 */
void hook_pswap_out(int16 procid, uint32 pagenum, uint32 framenum) {
#ifdef HOOK_LOG_ON
  // Based on the current policy, check if the page-number and framenumber being
  // replaced is accurate (ADIL)

  if (isbadpid(currpid)) {
    kprintf("[ERR] bad pid\n");
    panic("FAIL\n");
  }

	kprintf("\n=== Page Replacement  ===\n");
  kprintf("=== Frame: %d, Process-id: %d, Page-Number: %d ===\n", framenum, procid, pagenum);

#if POLICY_TESTING == 1
  if (currpolicy == FIFO) {
    kprintf("Testing FIFO .. ");


    //sid:
    //frame_md.reclaimframe=framenum;

    int16 recf = frame_md.reclaimframe;
    uint32 correct_pid = frame_md.alloc_page_proc[recf];
    uint32 correct_page = frame_md.alloc_page[recf];
    if (correct_page != pagenum || procid != correct_pid) {
      kprintf("[ERR] policy is not being followed\n");
      panic("FAIL\n");
    }
    frame_md.reclaimframe++;
    if (frame_md.reclaimframe == NFRAMES) frame_md.reclaimframe = 0;

    kprintf("FIFO Passed\n");
  } 
  else {
    kprintf("Testing GCA .. ");

    // If you decide to complete GCA part, you can test here.

    int32 i = (lastfm+1)%NFRAMES;

    uint32 vd;
    uint32 vpn;

    vd_t *vdptr;
    pd_t *pdptr;
    pt_t *ptptr;

    uint32 pd_offset;
    uint32 pt_offset;
    //uint32 pg_offset;


    while(i != framenum){
      if(frame_tab[i].type==FRAME_PR){
        vpn = inverted_page_tab[i].vpn;
        vd = vpn_to_address(vpn);      
        vdptr = (vd_t *)(&vd);

        pd_offset=vdptr->pd_offset;
        pt_offset=vdptr->pt_offset;
        //pg_offset=vdptr->pg_offset;

        pdptr = proctab[procid].prpd;
    
        ptptr = (pt_t*)vpn_to_address(pdptr[pd_offset].pd_base);
        
        if(ptptr[pt_offset].pt_acc == 0 && ptptr[pt_offset].pt_dirty != 0){
          panic("FAILure others\n");
        }
      }
      

      i=(i+1)%NFRAMES;
    }
    
    vd = vpn_to_address(pagenum);
    vdptr = (vd_t *)(&vd);
    pdptr = proctab[procid].prpd;

    // pd_offset=vdptr->pd_offset;
    // pt_offset=vdptr->pt_offset;
    // //pg_offset=vdptr->pg_offset;

    // pdptr+=pd_offset;
    // ptptr = (pt_t*)vpn_to_address(pdptr->pd_base);

    // ptptr+=pt_offset;

    // if(!(ptptr->pt_acc == 0 && ptptr->pt_dirty == 0)){
    //   XDEBUG_KPRINTF("FAILed to verify current %d, %d \n",ptptr->pt_acc,ptptr->pt_dirty );
    //   XDEBUG_KPRINTF("VPN %d, pres: %d, base %d\n",pagenum,ptptr->pt_pres,ptptr->pt_base);
    //   panic("FAILed to verify current\n" );
    // }

    pd_offset=vdptr->pd_offset;
    pt_offset=vdptr->pt_offset;
    //pg_offset=vdptr->pg_offset;

    ptptr = (pt_t*)vpn_to_address(pdptr[pd_offset].pd_base);


    if(!(ptptr[pt_offset].pt_acc == 0 && ptptr[pt_offset].pt_dirty == 0)){
      // XDEBUG_KPRINTF("FAILed to verify current %d, %d \n",ptptr[pt_offset].pt_acc,ptptr[pt_offset].pt_dirty );
      // XDEBUG_KPRINTF("VPN %d, pres: %d, base %d\n",pagenum,ptptr[pt_offset].pt_pres,ptptr[pt_offset].pt_base);
      panic("FAILed to verify current\n" );
    }

    lastfm = lframeNo;

    kprintf("Passed GCA\n");
  }
#endif


#endif
}
