#include <xinu.h>

#define PAGESIZE  4096
#define PAGE_ALLOCATION 100

/* NOTE: This does not check if paging is enabled or not, you
   should check that before starting the tests.
*/

/* Set to 1 to test page replacement
 * Set to 0 to check page fault handling is correct
 */
#define PAGE_REPLACEMENT 1

// Return a deterministic value per addr for testing.
uint32 get_test_value(uint32 *addr) {
  static uint32 v1 = 0x12345678;
  static uint32 v2 = 0xdeadbeef;
  return (uint32)addr + v1 + ((uint32)addr * v2);
}

static void do_policy_test(void) {
  uint32 npages = PAGE_ALLOCATION - 1;
  uint32 nbytes = npages * PAGESIZE;

  kprintf("Running Page Replacement Policy Test, with NFRAMES = %d\n", NFRAMES);

  char *mem = vgetmem(nbytes);
  if (mem == (char*) SYSERR) {
    panic("Page Replacement Policy Test failed\n");
    return;
  }

  // Write data
  for (uint32 i = 0; i<npages; i++) {
    uint32 *p = (uint32*)(mem + (i * PAGESIZE));

    // kprintf("Write Iteration [%3d] at 0x%08x\n", i, p);
    for (uint32 j=0; j<PAGESIZE; j=j+4) {
      uint32 v = get_test_value(p);
      *p++ = v;
    }

    sleepms(20); // to make it slower
  }

  // Check the data was truly written
  for (uint32 i = 0; i<npages; i++) {
    uint32 *p = (uint32*)(mem + (i * PAGESIZE));
    kprintf("Check Iteration [%3d] at 0x%08x\n", i, p);
    for (uint32 j=0; j<PAGESIZE; j=j+4) {
      uint32 v = get_test_value(p);
      ASSERT(*p++ == v);
    }

    sleepms(20); // to make it slower
  }

  if (vfreemem(mem, nbytes) == SYSERR) {
    panic("Policy Test: vfreemem() failed.\n");
  } else {
#if PAGE_REPLACEMENT == 1
    kprintf("\nPage Replacement Policy Test Finished.\n");
#else
    kprintf("\nPage Fault Handling Test Finished\n");
#endif
    kprintf("Here NFRAMES = %d\n", NFRAMES);
  }
}

/**
 * Just iterate through a lot of pages, and check if the output satisfies the policy.
 * Based on the hooks: hook_pfault and hook_pswap_out, you can ascertain if the test
 * passes or not. The hooks are supposed to throw a panic if the policy is not being
 * followed. (NFRAMES should be <= 50 for page replacement testing)
 */
void page_policy_test(void) {
  recvclr();

#if PAGE_REPLACEMENT == 1
  kprintf("Starting Policy (FIFO) Testing Test\n");
  if (NFRAMES > 50) {
    kprintf("Test Failed. Please set NFRAMES to <= 50 and try again.\n");
    return;
  }
#else
  kprintf("Starting Page Fault Handling Test\n");
  if (NFRAMES < 200) {
    kprintf("Test Failed. Please set NFRAMES to >= 200 and try again\n");
    return;
  }
#endif

  pid32 p = vcreate(do_policy_test, INITSTK, PAGE_ALLOCATION,
                    INITPRIO, "page rep", 0, NULL);
  resume(p);

  while (1) {
    if(proctab[p].prstate == PR_FREE) {
      break;
    }
    else {
      sleepms(100);
    }
  }

  kprintf("\n\nTest Passed %d.\n\n",get_faults());

  return;
}


static void do_policy_test_fifo_gca(void) {
  uint32 npages = PAGE_ALLOCATION - 1;
  uint32 nbytes = npages * PAGESIZE;
  uint32 oldframe=3;

  kprintf("Running Page Replacement Policy Test, with NFRAMES = %d old frame %d\n", NFRAMES,oldframe);

  char *mem = vgetmem(nbytes);
  if (mem == (char*) SYSERR) {
    panic("Page Replacement Policy Test failed\n");
    return;
  }

  // Write data
  for (uint32 i = 0; i<npages; i++) {
    uint32 *p = (uint32*)(mem + (i * PAGESIZE));
    // kprintf("Write Iteration [%3d] at 0x%08x\n", i, p);
    //access a new page
    uint32 v = get_test_value(p);
    *p = v;
    //access a old one
    int j = i % oldframe;
    p = (uint32*)(mem+(j * PAGESIZE));
    v = get_test_value(p);
    *p = v;
    sleepms(20); // to make it slower
  }

  if (vfreemem(mem, nbytes) == SYSERR) {
    panic("Policy Test: vfreemem() failed.\n");
  } else {
#if PAGE_REPLACEMENT == 1
    kprintf("\nPage Replacement Policy Test Finished.\n");
#else
    kprintf("\nPage Fault Handling Test Finished\n");
#endif
    kprintf("Here NFRAMES = %d\n", NFRAMES);
  }

  kprintf("Fifo,gca passed %d\n",get_faults());
}


void page_policy_test_fifo_gca(int numproc) {
  recvclr();

#if PAGE_REPLACEMENT == 1
  kprintf("Starting Policy (FIFO) Testing Test with proces %d\n",numproc);
  if (NFRAMES > 50) {
    kprintf("Test Failed. Please set NFRAMES to <= 50 and try again.\n");
    return;
  }
#else
  kprintf("Starting Page Fault Handling Test\n");
  if (NFRAMES < 200) {
    kprintf("Test Failed. Please set NFRAMES to >= 200 and try again\n");
    return;
  }
#endif

  int pids[numproc];

  resched_cntl(DEFER_START);
  for(int i=0;i<numproc;i++){
    pids[i] = vcreate(do_policy_test_fifo_gca, INITSTK, PAGE_ALLOCATION,
                    INITPRIO, "page policy", 0, NULL);  
    resume(pids[i]);
  }
  
  resched_cntl(DEFER_STOP);
  int flag=0;
  while (1) {
    flag=0;
    for(int i=0;i<numproc;i++){
      if(proctab[pids[i]].prstate != PR_FREE) {  
         flag=1;
         break; 
      }
    }

    if(flag==1){
      sleepms(100);
    }
    else{
      break;
    }
  }

  kprintf("\n\n All finished %d.\n\n",get_faults());

  return;
}


// static void do_policy_test(void) {
//   uint32 npages = PAGE_ALLOCATION - 1;
//   uint32 nbytes = npages * PAGESIZE;

//   kprintf("Running Page Replacement Policy Test, with NFRAMES = %d\n", NFRAMES);

//   char *mem = vgetmem(nbytes);
//   if (mem == (char*) SYSERR) {
//     panic("Page Replacement Policy Test failed\n");
//     return;
//   }

//   // Write data
//   for (uint32 i = 0; i<npages; i++) {
//     uint32 *p = (uint32*)(mem + (i * PAGESIZE));

//     // kprintf("Write Iteration [%3d] at 0x%08x\n", i, p);
//     for (uint32 j=0; j<PAGESIZE; j=j+4) {
//       uint32 v = get_test_value(p);
//       *p++ = v;

//       if(*(p-1) != v){
//          uint32 vd = vpn_to_address(address_to_vpn(p));
//           vd_t *vdptr = (vd_t *)(&vd);
//           pd_t *pdptr = proctab[currpid].prpd;

//           uint32 pd_offset=vdptr->pd_offset;
//           uint32 pt_offset=vdptr->pt_offset;
//           //pg_offset=vdptr->pg_offset;

//           pt_t* ptptr = (pt_t*)vpn_to_address(pdptr[pd_offset].pd_base);
//           // XDEBUG_KPRINTF("VPN %d, pres: %d, base %d\n",pagenum,ptptr[pt_offset].pt_pres,ptptr[pt_offset].pt_base);
      
//           XDEBUG_KPRINTF("Procid %d, addr %d, vpn %d, frame %d, \n",currpid,p,((uint32)p)/NBPG,ptptr[pt_offset].pt_base);

//         panic("writing assert failed\n");
//       }
//     }

//     sleepms(20); // to make it slower
//   }

//   // Check the data was truly written
//   for (uint32 i = 0; i<npages; i++) {
//     uint32 *p = (uint32*)(mem + (i * PAGESIZE));
//     kprintf("Check Iteration [%3d] at 0x%08x\n", i, p);
//     for (uint32 j=0; j<PAGESIZE; j=j+4) {
//       uint32 v = get_test_value(p);
//       if(*p++ != v){
//          uint32 vd = vpn_to_address(address_to_vpn(p));
//           vd_t *vdptr = (vd_t *)(&vd);
//           pd_t *pdptr = proctab[currpid].prpd;

//           uint32 pd_offset=vdptr->pd_offset;
//           uint32 pt_offset=vdptr->pt_offset;
//           //pg_offset=vdptr->pg_offset;

//           pt_t* ptptr = (pt_t*)vpn_to_address(pdptr[pd_offset].pd_base);
//           // XDEBUG_KPRINTF("VPN %d, pres: %d, base %d\n",pagenum,ptptr[pt_offset].pt_pres,ptptr[pt_offset].pt_base);
      
//           XDEBUG_KPRINTF("Procid %d, addr %d, vpn %d, frame %d, \n",currpid,p,((uint32)p)/NBPG,ptptr[pt_offset].pt_base);

//         panic("assert failed\n");
//       }
//       //ASSERT(*p++ == v);
//     }

//     sleepms(20); // to make it slower
//   }

//   if (vfreemem(mem, nbytes) == SYSERR) {
//     panic("Policy Test: vfreemem() failed.\n");
//   } else {
// #if PAGE_REPLACEMENT == 1
//     kprintf("\nPage Replacement Policy Test Finished.\n");
// #else
//     kprintf("\nPage Fault Handling Test Finished\n");
// #endif
//     kprintf("Here NFRAMES = %d\n", NFRAMES);
//   }
// }

// /**
//  * Just iterate through a lot of pages, and check if the output satisfies the policy.
//  * Based on the hooks: hook_pfault and hook_pswap_out, you can ascertain if the test
//  * passes or not. The hooks are supposed to throw a panic if the policy is not being
//  * followed. (NFRAMES should be <= 50 for page replacement testing)
//  */
// void page_policy_test(void) {
//   recvclr();

// #if PAGE_REPLACEMENT == 1
//   kprintf("Starting Policy (FIFO) Testing Test\n");
//   if (NFRAMES > 50) {
//     kprintf("Test Failed. Please set NFRAMES to <= 50 and try again.\n");
//     return;
//   }
// #else
//   kprintf("Starting Page Fault Handling Test\n");
//   if (NFRAMES < 200) {
//     kprintf("Test Failed. Please set NFRAMES to >= 200 and try again\n");
//     return;
//   }
// #endif

//   pid32 p = vcreate(do_policy_test, INITSTK, PAGE_ALLOCATION,
//                     INITPRIO, "page rep", 0, NULL);
//   resume(p);

//   while (1) {
//     if(proctab[p].prstate == PR_FREE) {
//       break;
//     }
//     else {
//       sleepms(100);
//     }
//   }

//   kprintf("Page policy test Page faults:-> %d\n",get_faults());
//   kprintf("\n\nTest Passed.\n\n");

//   return;
// }


//1/2 vgetmem 1/2 normal getmem
void mytest1(void){
    uint32 npages = PAGE_ALLOCATION - 1;
    npages=npages/2;
    uint32 nbytes = npages * PAGESIZE;
    uint32 memnpages=2;
    uint32 memnbytes=memnpages*PAGESIZE;

    kprintf("Running Customize Page Replacement Policy Test, with NFRAMES = %d\n", NFRAMES);

    char *mem = vgetmem(nbytes);
    if (mem == (char*) SYSERR) {
      panic("vmeme allocation failed\n");
      return;
    }
    printXMemlist(currpid);

    char *mem2 = getmem(memnbytes);
    if (mem2 == (char*) SYSERR) {
      panic("Normal memory allocation failed\n");
      return;
    }

    // Write data
    for (uint32 i = 0; i<npages; i++) {
      uint32 *p = (uint32*)(mem + (i * PAGESIZE));

      // kprintf("Write Iteration [%3d] at 0x%08x\n", i, p);
      for (uint32 j=0; j<PAGESIZE; j=j+4) {
        uint32 v = get_test_value(p);
        *p++ = v;

        if(*(p-1) != v){
         uint32 vd = vpn_to_address(address_to_vpn(p));
          vd_t *vdptr = (vd_t *)(&vd);
          pd_t *pdptr = proctab[currpid].prpd;

          uint32 pd_offset=vdptr->pd_offset;
          uint32 pt_offset=vdptr->pt_offset;
          //pg_offset=vdptr->pg_offset;

          pt_t* ptptr = (pt_t*)vpn_to_address(pdptr[pd_offset].pd_base);
          // XDEBUG_KPRINTF("VPN %d, pres: %d, base %d\n",pagenum,ptptr[pt_offset].pt_pres,ptptr[pt_offset].pt_base);
      
          XDEBUG_KPRINTF("Procid %d, addr %d, vpn %d, frame %d, \n",currpid,p,((uint32)p)/NBPG,ptptr[pt_offset].pt_base);

        panic("writing assert failed\n");
        }
      }

      sleepms(20); // to make it slower
    }

    // Write data
    for (uint32 i = 0; i<memnpages; i++) {
      uint32 *p = (uint32*)(mem2 + (i * PAGESIZE));

      // kprintf("Write Iteration [%3d] at 0x%08x\n", i, p);
      for (uint32 j=0; j<PAGESIZE; j=j+4) {
        uint32 v = get_test_value(p);
        *p++ = v;

        if(*(p-1) != v){
         uint32 vd = vpn_to_address(address_to_vpn(p));
          vd_t *vdptr = (vd_t *)(&vd);
          pd_t *pdptr = proctab[currpid].prpd;

          uint32 pd_offset=vdptr->pd_offset;
          uint32 pt_offset=vdptr->pt_offset;
          //pg_offset=vdptr->pg_offset;

          pt_t* ptptr = (pt_t*)vpn_to_address(pdptr[pd_offset].pd_base);
          // XDEBUG_KPRINTF("VPN %d, pres: %d, base %d\n",pagenum,ptptr[pt_offset].pt_pres,ptptr[pt_offset].pt_base);
      
          XDEBUG_KPRINTF("Procid %d, addr %d, vpn %d, frame %d, \n",currpid,p,((uint32)p)/NBPG,ptptr[pt_offset].pt_base);

          panic("writing assert failed\n");
        }
      }

      sleepms(20); // to make it slower
    }

    // Check the data was truly written
    for (uint32 i = 0; i<npages; i++) {
      uint32 *p = (uint32*)(mem + (i * PAGESIZE));
      kprintf("Check Iteration [%3d] at 0x%08x\n", i, p);
      for (uint32 j=0; j<PAGESIZE; j=j+4) {
        uint32 v = get_test_value(p);
        
        if(*p++ != v){
        
          uint32 vd = vpn_to_address(address_to_vpn(p));
          vd_t *vdptr = (vd_t *)(&vd);
          pd_t *pdptr = proctab[currpid].prpd;

          uint32 pd_offset=vdptr->pd_offset;
          uint32 pt_offset=vdptr->pt_offset;
          //pg_offset=vdptr->pg_offset;

          pt_t* ptptr = (pt_t*)vpn_to_address(pdptr[pd_offset].pd_base);
          // XDEBUG_KPRINTF("VPN %d, pres: %d, base %d\n",pagenum,ptptr[pt_offset].pt_pres,ptptr[pt_offset].pt_base);
      
          XDEBUG_KPRINTF("Procid %d, addr %d, vpn %d, frame %d, \n",currpid,p,((uint32)p)/NBPG,ptptr[pt_offset].pt_base);


          panic("assert failed\n");
        }
      //ASSERT(*p++ == v);
      }

      sleepms(20); // to make it slower
    }

    kprintf("Verify normal memwriting\n");
    // Check the data was truly written
    for (uint32 i = 0; i<memnpages; i++) {
      uint32 *p = (uint32*)(mem2 + (i * PAGESIZE));
      kprintf("Check Iteration [%3d] at 0x%08x\n", i, p);
      for (uint32 j=0; j<PAGESIZE; j=j+4) {
        uint32 v = get_test_value(p);
        if(*p++ != v){
         uint32 vd = vpn_to_address(address_to_vpn(p));
          vd_t *vdptr = (vd_t *)(&vd);
          pd_t *pdptr = proctab[currpid].prpd;

          uint32 pd_offset=vdptr->pd_offset;
          uint32 pt_offset=vdptr->pt_offset;
          //pg_offset=vdptr->pg_offset;

          pt_t* ptptr = (pt_t*)vpn_to_address(pdptr[pd_offset].pd_base);
          // XDEBUG_KPRINTF("VPN %d, pres: %d, base %d\n",pagenum,ptptr[pt_offset].pt_pres,ptptr[pt_offset].pt_base);
      
          XDEBUG_KPRINTF("Procid %d, addr %d, vpn %d, frame %d, \n",currpid,p,((uint32)p)/NBPG,ptptr[pt_offset].pt_base);

        panic("assert failed\n");
      }
      //ASSERT(*p++ == v);
      }

      sleepms(20); // to make it slower
    }

    if (vfreemem(mem, nbytes) == SYSERR || freemem(mem2,memnbytes)==SYSERR) {
      panic("Policy Test: vfreemem() or freemem failed.\n");
    }   
    else {
      #if PAGE_REPLACEMENT == 1
          kprintf("\nPage Replacement Policy Test Finished.\n");
      #else
          kprintf("\nPage Fault Handling Test Finished\n");
      #endif
          kprintf("Here NFRAMES = %d\n", NFRAMES);
    }
    printXMemlist(currpid);
}

#define DIVISION 10

//process has multipel vgetmem
void mytest2(void){
    uint32 npages = PAGE_ALLOCATION - 1;
    npages=npages/DIVISION;
    uint32 nbytes = npages * PAGESIZE;

    kprintf("Running Customize Page Replacement Policy Test, with NFRAMES = %d\n", NFRAMES);

    char* mems[DIVISION];

    XDEBUG_KPRINTF("Getting all memory by parts\n");

    for(int i=0;i<DIVISION;i++){
        //get a segement
        char *mem = vgetmem(nbytes);  
        if (mem == (char*) SYSERR) {
          panic("vmeme allocation failed\n");
          return;
        }

        mems[i]=mem;
        printXMemlist(currpid);
    }

    XDEBUG_KPRINTF("Writing on all parts\n");

    for(int i=0;i<DIVISION;i++){
      // Write data
      for (uint32 i = 0; i<npages; i++) {
        uint32 *p = (uint32*)(mems[i] + (i * PAGESIZE));

        // kprintf("Write Iteration [%3d] at 0x%08x\n", i, p);
        for (uint32 j=0; j<PAGESIZE; j=j+4) {
          uint32 v = get_test_value(p);
          *p++ = v;

          if(*(p-1) != v){
         uint32 vd = vpn_to_address(address_to_vpn(p));
          vd_t *vdptr = (vd_t *)(&vd);
          pd_t *pdptr = proctab[currpid].prpd;

          uint32 pd_offset=vdptr->pd_offset;
          uint32 pt_offset=vdptr->pt_offset;
          //pg_offset=vdptr->pg_offset;

          pt_t* ptptr = (pt_t*)vpn_to_address(pdptr[pd_offset].pd_base);
          // XDEBUG_KPRINTF("VPN %d, pres: %d, base %d\n",pagenum,ptptr[pt_offset].pt_pres,ptptr[pt_offset].pt_base);
      
          XDEBUG_KPRINTF("Procid %d, addr %d, vpn %d, frame %d, \n",currpid,p,((uint32)p)/NBPG,ptptr[pt_offset].pt_base);

        panic("writing assert failed\n");
      }
        }

        sleepms(20); // to make it slower
      }
    }

    XDEBUG_KPRINTF("Verifying all parts\n");
    
    for(int i=0;i<DIVISION;i++)   {
      // Check the data was truly written
      for (uint32 i = 0; i<npages; i++) {
        uint32 *p = (uint32*)(mems[i] + (i * PAGESIZE));
        kprintf("Check Iteration [%3d] at 0x%08x\n", i, p);
        for (uint32 j=0; j<PAGESIZE; j=j+4) {
          uint32 v = get_test_value(p);
          if(*p++ != v){
         uint32 vd = vpn_to_address(address_to_vpn(p));
          vd_t *vdptr = (vd_t *)(&vd);
          pd_t *pdptr = proctab[currpid].prpd;

          uint32 pd_offset=vdptr->pd_offset;
          uint32 pt_offset=vdptr->pt_offset;
          //pg_offset=vdptr->pg_offset;

          pt_t* ptptr = (pt_t*)vpn_to_address(pdptr[pd_offset].pd_base);
          // XDEBUG_KPRINTF("VPN %d, pres: %d, base %d\n",pagenum,ptptr[pt_offset].pt_pres,ptptr[pt_offset].pt_base);
      
          XDEBUG_KPRINTF("Procid %d, addr %d, vpn %d, frame %d, \n",currpid,p,((uint32)p)/NBPG,ptptr[pt_offset].pt_base);

        panic("assert failed\n");
      }
      //ASSERT(*p++ == v);
        }

        sleepms(20); // to make it slower
      }

    }
    
    XDEBUG_KPRINTF("Freeing odd ones\n");
    for(int i=0;i<DIVISION;i+=2){
        if(vfreemem(mems[i], nbytes) == SYSERR) {
          panic("Policy Test: vfreemem()\n");
        }
        printXMemlist(currpid);
    }

    XDEBUG_KPRINTF("Getting again odd ones\n");
    for(int i=0;i<DIVISION;i+=2){
        mems[i]=vgetmem(nbytes);
        if (mems[i] == (char*) SYSERR) {
          panic("vmeme allocation failed\n");
          return;
        }
        printXMemlist(currpid);
    }    

    XDEBUG_KPRINTF("Freeing All of them\n");

    for(int i=0;i<DIVISION;i++){
        if(vfreemem(mems[i], nbytes) == SYSERR) {
          panic("Policy Test: vfreemem()\n");
        }
        printXMemlist(currpid);
    }
    
    XDEBUG_KPRINTF("Test passed\n");
}

void mytest3(void){
    uint32 npages = PAGE_ALLOCATION-1;
    npages=npages/DIVISION;
    uint32 nbytes = npages * PAGESIZE;

    kprintf("Running Customize Page Replacement Policy Test, with NFRAMES = %d\n", NFRAMES);

    char* mems[DIVISION];

    XDEBUG_KPRINTF("Getting all memory by parts\n");

    for(int i=0;i<DIVISION;i++){
        
        XDEBUG_KPRINTF("Get mem %d\n",i);

        char *mem = vgetmem(nbytes);  
        if (mem == (char*) SYSERR) {
          panic("vmeme allocation failed\n");
          return;
        }

        mems[i]=mem;

        printXMemlist(currpid);

    }


    XDEBUG_KPRINTF("Writing on all parts\n");

    for(int i=0;i<DIVISION;i++){
      // Write data
      for (uint32 i = 0; i<npages; i++) {
        uint32 *p = (uint32*)(mems[i] + (i * PAGESIZE));

        // kprintf("Write Iteration [%3d] at 0x%08x\n", i, p);
        for (uint32 j=0; j<PAGESIZE; j=j+4) {
          uint32 v = get_test_value(p);
          *p++ = v;

        if(*(p-1) != v){
         uint32 vd = vpn_to_address(address_to_vpn(p));
          vd_t *vdptr = (vd_t *)(&vd);
          pd_t *pdptr = proctab[currpid].prpd;

          uint32 pd_offset=vdptr->pd_offset;
          uint32 pt_offset=vdptr->pt_offset;
          //pg_offset=vdptr->pg_offset;

          pt_t* ptptr = (pt_t*)vpn_to_address(pdptr[pd_offset].pd_base);
          // XDEBUG_KPRINTF("VPN %d, pres: %d, base %d\n",pagenum,ptptr[pt_offset].pt_pres,ptptr[pt_offset].pt_base);
      
          XDEBUG_KPRINTF("Procid %d, addr %d, vpn %d, frame %d, \n",currpid,p,((uint32)p)/NBPG,ptptr[pt_offset].pt_base);

        panic("writing assert failed\n");
      }
        }

        sleepms(20); // to make it slower
      }
    }

    XDEBUG_KPRINTF("Verifying all parts\n");
    
    for(int i=0;i<DIVISION;i++)   {
      // Check the data was truly written
      for (uint32 i = 0; i<npages; i++) {
        uint32 *p = (uint32*)(mems[i] + (i * PAGESIZE));
        //kprintf("Check Iteration [%3d] at 0x%08x\n", i, p);
        for (uint32 j=0; j<PAGESIZE; j=j+4) {
          uint32 v = get_test_value(p);
          if(*p++ != v){
             uint32 vd = vpn_to_address(address_to_vpn(p));
              vd_t *vdptr = (vd_t *)(&vd);
              pd_t *pdptr = proctab[currpid].prpd;

              uint32 pd_offset=vdptr->pd_offset;
              uint32 pt_offset=vdptr->pt_offset;
              //pg_offset=vdptr->pg_offset;

              pt_t* ptptr = (pt_t*)vpn_to_address(pdptr[pd_offset].pd_base);
              // XDEBUG_KPRINTF("VPN %d, pres: %d, base %d\n",pagenum,ptptr[pt_offset].pt_pres,ptptr[pt_offset].pt_base);
          
              XDEBUG_KPRINTF("Procid %d, addr %d, vpn %d, frame %d, \n",currpid,p,((uint32)p)/NBPG,ptptr[pt_offset].pt_base);

            panic("assert failed\n");
          }
      //ASSERT(*p++ == v);
        }

        sleepms(20); // to make it slower
      }

    }
    

    for(int i=0;i<DIVISION;i++){
        XDEBUG_KPRINTF("Freeing %d \n",i);    
        if(vfreemem(mems[i], nbytes) == SYSERR) {
          panic("Policy Test: vfreemem()\n");
        }

        printXMemlist(currpid);
    }

    XDEBUG_KPRINTF("Test passed\n");
}


static void do_policy_test1(void){
    XDEBUG_KPRINTF("Test 1: Vcreate has vgetmem and getmem\n");
    uint32 start=get_faults();
    mytest1();
    uint32 end=get_faults();    
    XDEBUG_KPRINTF("Test 1 Faults: -> %d",end-start);
   
}

static void do_policy_test2(void){
    XDEBUG_KPRINTF("Test 2: Vcreate calls multiple vgetmem\n");
    int32 start=get_faults();
    mytest2();
    int32 end=get_faults();    
    XDEBUG_KPRINTF("Test 2 Faults: -> %d",end-start);
   
}

static void do_policy_test3(void){
    XDEBUG_KPRINTF("Test 3: Vcreate calls multiple vgetmem\n");
    int32 start=get_faults();
    mytest3();
    int32 end=get_faults();    
    XDEBUG_KPRINTF("Test 3 Faults: -> %d",end-start);
}


void page_policy_test_methods(int testno){
  recvclr();

  #if PAGE_REPLACEMENT == 1
  kprintf("Starting Policy (FIFO) Testing Test\n");
  if (NFRAMES > 50) {
    kprintf("Test Failed. Please set NFRAMES to <= 50 and try again.\n");
    return;
  }
  #else
    kprintf("Starting Page Fault Handling Test\n");
    if (NFRAMES < 200) {
      kprintf("Test Failed. Please set NFRAMES to >= 200 and try again\n");
      return;
    }
  #endif

  pid32 p;

  XDEBUG_KPRINTF("Creating process id %d\n",p);
  if(testno==1){
    p = vcreate(do_policy_test1, INITSTK, PAGE_ALLOCATION,INITPRIO, "page rep test 1", 0, NULL);
    resume(p);
  }
  else if(testno==2){
    p = vcreate(do_policy_test2, INITSTK, PAGE_ALLOCATION,INITPRIO, "page rep test 2", 0, NULL);
    resume(p);
  }
  else if(testno==3){
    p = vcreate(do_policy_test3, INITSTK, PAGE_ALLOCATION,INITPRIO, "page rep test 3", 0, NULL);
    resume(p);
  }
  else{
      panic("not defined yet");
  }
  XDEBUG_KPRINTF("Process id %d\n",p);

  while (1) {
    if(proctab[p].prstate == PR_FREE) {
      break;
    }
    else {
      sleepms(100);
    }
  }

  kprintf("\n\nTest %d Passed.\n\n",testno);

  XDEBUG_KPRINTF("Method %d faults %d\n",testno,get_faults());

}


void page_policy_test_custom(void) {
  
  page_policy_test_methods(1);
  page_policy_test_methods(2);
  page_policy_test_methods(3);

  return;
}