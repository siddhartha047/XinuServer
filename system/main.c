/*  main.c  - main */

#include <xinu.h>
#include<string.h>

pid32 prA, prB, prC;

int proc_a(char);
int proc_b(char);
int cpubound(char);
int iobound(char);

volatile int a_cnt = 0;
volatile int b_cnt = 0;
volatile int LOOP1 = 0;
volatile int LOOP2 = 0;

void test4(void);
void test3(void);
void test12(void);

process	main(void)
{

	test12();
	//test3();
	//test4();
	
	return OK;
    
}

void test12(void){

	recvclr();

	int cpu=6;
	int io=0;

	pid32 pidA[cpu+1];
	pid32 pidB[io+1];

	char cpunames[6][6]={"CPU-1","CPU-2","CPU-3","CPU-4","CPU-5","CPU-6"};
	char ionames[6][5]={"IO-1","IO-2","IO-3","IO-4","IO-5","IO-6"};


	LOOP1=100;
	LOOP2=100;

	for(int i=0;i<cpu;i++){		
		pidA[i]=create(cpubound, 2000, SRTIME, 1+i*3,cpunames[i] , 1, 'A'+i);							
	}	
	for(int i=0;i<io;i++){		
		pidB[i]=create(iobound, 2000, SRTIME, 1+i*3,ionames[i] , 1, 'a'+i);				
	}	

	//resched_cntl(DEFER_START);

	for(int i=0;i<cpu;i++){
		resume(pidA[i]);
	}	
	for(int i=0;i<io;i++){	
		resume(pidB[i]);
	}	

	//resched_cntl(DEFER_STOP);
	sleep(10);
	chgprio(SRTIME,20);

	while(1){};
	sleep(10);

	
	for(int i=0;i<cpu;i++){
		kill(pidA[i]);
	}	
	for(int i=0;i<io;i++){	
		kill(pidB[i]);
	}	



	intmask mask;    
	mask = disable();
	XTEST_KPRINTF("\nTest Result: A = %d, B = %d\n", a_cnt, b_cnt);
	restore(mask);

}


void test3(void){		
	recvclr();

	int cpu=6;
	int io=6;

	pid32 pidA[cpu+1];
	pid32 pidB[io+1];

	char cpunames[6][6]={"CPU-1","CPU-2","CPU-3","CPU-4","CPU-5","CPU-6"};
	char ionames[6][5]={"IO-1","IO-2","IO-3","IO-4","IO-5","IO-6"};



	LOOP1=1000;
	LOOP2=100;

	for(int i=0;i<cpu;i++){		
		//pidA[i]=create(cpubound, 2000, SRTIME, 1+i*3,cpunames[i] , 1, 'A'+i);					
		pidA[i]=create(cpubound, 2000, TSSCHED, 1+i*3,cpunames[i] , 1, '1'+i);					
	}	
	for(int i=0;i<io;i++){		
		//pidB[i]=create(iobound, 2000, SRTIME, 1+i*3,ionames[i] , 1, 'a'+i);		
		pidB[i]=create(iobound, 2000, TSSCHED, 1+i*3,ionames[i] , 1, '1'+i);		
	}	

	resched_cntl(DEFER_START);

	for(int i=0;i<cpu;i++){
		resume(pidA[i]);
	}	
	for(int i=0;i<io;i++){	
		resume(pidB[i]);
	}	

	resched_cntl(DEFER_STOP);

	sleep(10);

	
	for(int i=0;i<cpu;i++){
		kill(pidA[i]);
	}	
	for(int i=0;i<io;i++){	
		kill(pidB[i]);
	}	



	intmask mask;    
	mask = disable();
	XTEST_KPRINTF("\nTest Result: A = %d, B = %d\n", a_cnt, b_cnt);
	restore(mask);

}

void test4(void){

	recvclr();

	int cpu=6;
	int io=0;

	pid32 pidA[cpu+1];
	pid32 pidB[io+1];

	char cpunames[6][6]={"CPU-1","CPU-2","CPU-3","CPU-4","CPU-5","CPU-6"};
	char ionames[6][5]={"IO-1","IO-2","IO-3","IO-4","IO-5","IO-6"};

	LOOP1=1000;
	LOOP2=100;

	resched_cntl(DEFER_START);

	pid32 root=create(cpubound, 2000, TSSCHED, 1 ,'root process' , 1,'1');						
	pid32 child1=create(cpubound, 2000, TSSCHED, 1 ,'Child 1' , 1, '1');							
	pid32 child2=create(cpubound, 2000, TSSCHED, 1 ,'Child 2' , 1, '1');						

		
	resched_cntl(DEFER_START);

	resume(root);
	resume(child1);
	resume(child2);

	resched_cntl(DEFER_STOP);	

	intmask mask;    
	mask = disable();
	XTEST_KPRINTF("End of main process\n");
	restore(mask);
	while(1){};
}

int proc_a(char ch){	
	XTEST_KPRINTF("Starting Process = %c...\n",ch);  		
	while(1){
		for(int i=0;i<10000;i++);		
		a_cnt++;	
		//sleepms(100);	
	}
	return 0;
}

int proc_b(char ch){	
	XTEST_KPRINTF("Starting Process = %c...\n",ch);  		
	while(1){
		for(int i=0;i<10000;i++);		
		b_cnt++;	
		//sleepms(100);	
	}
	return 0;
}

int cpubound(char ch){	

	intmask mask;    
	mask = disable();
	XTEST_KPRINTF("Starting CPU Process = %c...\n",ch);  	
	restore(mask);

	for (int i=0; i<LOOP1; i++) {
		for (int j=0; j<LOOP2; j++) {
			for(int k=0;k<10000;k++);		
			a_cnt++;			
		}
		mask = disable();
		XTEST_KPRINTF("[CPU->%c : Pid->%d, i->%d, prio->%d, preempt->%d, quantum->%d\n",ch,currpid,i,(&proctab[currpid])->prprio,preempt,(&proctab[currpid])->pr_quantum);
		restore(mask);

		// Using kprintf print the pid followed the outer loop count i,
		// the process's priority and remaining time slice (preempt).
	}
	
	return 0;
}

int iobound(char ch){	
	intmask mask;    
	mask = disable();
	XTEST_KPRINTF("Starting IO Process = %c...\n",ch);  		
	restore(mask);
	
	for (int i=0; i<LOOP1; i++) {
		for (int j=0; j<LOOP2; j++) {
			for(int k=0;k<10000;k++);		
			b_cnt++;			
			sleepms(10);
		}

		mask = disable();
		XTEST_KPRINTF("[IO->%c : Pid->%d, i->%d, prio->%d, preempt->%d, quantum->%d\n",ch,currpid,i,(&proctab[currpid])->prprio,preempt,(&proctab[currpid])->pr_quantum);
		restore(mask);
	}
	return 0;
}


// recvclr();
// 	pid32 processId=create(shell, 4096, 20, "shell", 1, CONSOLE);		
// 	XTEST_KPRINTF("Spawning new shell with PID = %d...\n",processId);  
// 	resume(processId);
	
// 	/* Wait for shell to exit and recreate it */	

// 	while (TRUE) {
// 		receive();
// 		sleepms(200);				
// 		pid32 processId=create(shell, 4096, 20, "shell", 1, CONSOLE);		
// 		XTEST_KPRINTF("Spawning new shell with PID = %d...\n",processId);  
// 		resume(processId);
// 	}
// 	return OK;