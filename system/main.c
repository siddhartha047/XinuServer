#include <xinu.h>

extern void page_policy_test(void);
extern void page_policy_test_custom(void);

void test0(void);
void cpubound(char ch);
void A(char ch);
void test1(void);
void given_test(char ch);
void custom_test(char ch);
void test2(void);

process	main(void)
{
  srpolicy(FIFO);
  //srpolicy(GCA);

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
  
  // page_policy_test();
  // XDEBUG_KPRINTF("Page Faults: -> %d\n",get_faults());
  
  //test0();
  //test1();
  
  test2();
  


  XTEST_KPRINTF("Main process ending\n");


  return OK;
}

void test0(void){
  int prA=create( A, 2000, INITPRIO, "A", 1,'A' );
  resume(prA);
}



void test1(void){
  resched_cntl(DEFER_START);  
    int prA=create( A, 2000, INITPRIO, "A", 1,'A' );
    int prTest=create(given_test, 2000, INITPRIO, "A", 1,'B' );
    int prTestCustom=create(custom_test, 2000, INITPRIO, "C", 1,'A' );
    resume(prA);
    resume(prTest);
    resume(prTestCustom);
  resched_cntl(DEFER_STOP);
}

void test2(void){

  resched_cntl(DEFER_START);  
    int prTest=create(given_test, 2000, INITPRIO, "A", 1,'1' );
    int prTestCustom=create(given_test, 2000, INITPRIO, "C", 1,'2' );
    resume(prTest);
    resume(prTestCustom);
  resched_cntl(DEFER_STOP);
}


void given_test(char ch){
  page_policy_test();  
  
  XDEBUG_KPRINTF("Policy test Page Faults: -> %d\n",get_faults());
  XTEST_KPRINTF("Policy test ending\n",ch);

}

void custom_test(char ch){
  page_policy_test_custom();
  
  XDEBUG_KPRINTF("Policy test custom Page Faults: -> %d\n",get_faults());
  XTEST_KPRINTF("Policy test custom ending\n",ch);

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