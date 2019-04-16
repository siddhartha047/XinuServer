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

  kprintf("\n\nTest Passed.\n\n");

  return;
}


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

    kprintf("Verify normal memwriting\n");
    // Check the data was truly written
    for (uint32 i = 0; i<memnpages; i++) {
      uint32 *p = (uint32*)(mem2 + (i * PAGESIZE));
      kprintf("Check Iteration [%3d] at 0x%08x\n", i, p);
      for (uint32 j=0; j<PAGESIZE; j=j+4) {
        uint32 v = get_test_value(p);
        ASSERT(*p++ == v);
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
          ASSERT(*p++ == v);
        }

        sleepms(20); // to make it slower
      }

    }
    
    XDEBUG_KPRINTF("Freeing odd ones\n");
    for(int i=0;i<DIVISION;i+=2){
        if(vfreemem(mems[i], nbytes) == SYSERR) {
          panic("Policy Test: vfreemem()\n");
        }
    }

    XDEBUG_KPRINTF("Getting again odd ones\n");
    for(int i=0;i<DIVISION;i+=2){
        mems[i]=vgetmem(nbytes);
        if (mems[i] == (char*) SYSERR) {
          panic("vmeme allocation failed\n");
          return;
        }
    }    

    XDEBUG_KPRINTF("Freeing All of them\n");

    for(int i=0;i<DIVISION;i++){
        if(vfreemem(mems[i], nbytes) == SYSERR) {
          panic("Policy Test: vfreemem()\n");
        }
    }
    
    XDEBUG_KPRINTF("Test passed\n");
}


static void do_policy_test_custom(void) {
    XDEBUG_KPRINTF("Test 1: Vcreate has vgetmem and getmem\n");
    mytest1();    
    
    sleep(2);

    XDEBUG_KPRINTF("Test 2: Vcreate calls multiple vgetmem\n");
    mytest2();
}



void page_policy_test_custom(void) {
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

  pid32 p = vcreate(do_policy_test_custom, INITSTK, PAGE_ALLOCATION,
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

  kprintf("\n\nTest Passed.\n\n");

  return;
}
