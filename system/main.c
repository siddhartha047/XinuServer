#include <xinu.h>

extern void page_policy_test(void);

void test0(void);
void cpubound(char ch);
void A(char ch);

process	main(void)
{
  srpolicy(FIFO);

  /* Start the network */
  /* DO NOT REMOVE OR COMMENT BELOW */
#ifndef QEMU
  netstart();
#endif

  /*  TODO. Please ensure that your paging is activated 
      by checking the values from the control register.
  */

  /* Initialize the page server */
  /* DO NOT REMOVE OR COMMENT THIS CALL */
  psinit();

  //test0();
  page_policy_test();
  //page_policy_test_custom();
  

  XTEST_KPRINTF("Main process ending\n");


  return OK;
}

void test0(void){
  int prA=create( A, 2000, 50, "A", 1,'A' );

  resume(prA);
}


void cpubound(char ch){
  for(int i=0;i<15;i++){
    XTEST_KPRINTF("(%c Tasks %d)->",ch,i);
    for(int k=0;k<100;k++){     
      for(int j=0;j<1000000;j++);       
      //sleep(1);
    }
    XTEST_KPRINTF("<-(%c Tasks %d)\n",ch,i);    
  }   
}


void A(char ch)
{
  
  cpubound(ch);
  XTEST_KPRINTF("%c ending\n",ch);

  return;
}