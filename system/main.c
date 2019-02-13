/*  main.c  - main */

#include <xinu.h>

pid32 prA, prB, prC;

int proc_a(char), proc_b(char), proc_c(char);
volatile int a_cnt = 0;
volatile int b_cnt = 0;
volatile int c_cnt = 0;

process	main(void)
{

	recvclr();

	/*prA = create(proc_a, 2000, SRTIME, 30, "proc A", 1, 'A');
	prB = create(proc_b, 2000, TSSCHED, 20, "proc B", 1, 'B');
	prC = create(proc_c, 2000, TSSCHED, 10, "proc C", 1, 'C');
	
	resume(prA);
	resume(prB);
	resume(prC);
	
	sleepms(100);
	
	kill(prA);
	kill(prB);
	kill(prC);

	XTEST_KPRINTF("\nTest Result: A = %d, B = %d, C = %d\n", a_cnt, b_cnt, c_cnt);*/

	int procesNo=10;
	pid32 pidA[procesNo];
	pid32 pidB[procesNo];


	for(int i=0;i<procesNo;i++){
		pidA[i]=create(proc_a, 2000, SRTIME, 20, "proc A", 1, 'A');
		pidB[i]=create(proc_b, 2000, TSSCHED, 10, "proc B", 1, 'B');		
	}	

	for(int i=0;i<procesNo;i++){
		resume(pidA[i]);
		resume(pidB[i]);
	}

	sleepms(100);

	for(int i=0;i<procesNo;i++){
		kill(pidA[i]);
		kill(pidB[i]);
	}

	XTEST_KPRINTF("\nTest Result: A = %d, B = %d\n", a_cnt, b_cnt);

	
	return OK;
    
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

int proc_c(char ch){	
	XTEST_KPRINTF("Starting Process = %c...\n",ch);  		
	while(1){
		for(int i=0;i<10000;i++);		
		c_cnt++;	
		//sleepms(100);	 	
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