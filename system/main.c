/*  main.c  - main */

#include <xinu.h>

process	main(void)
{

	/* Run the Xinu shell */	
	//Task 2

	recvclr();
	pid32 processId=create(shell, 4096, 20, "shell", 1, CONSOLE);		
	XTEST_KPRINTF("Spawning new shell with PID = %d...\n",processId);  
	resume(processId);
	
	/* Wait for shell to exit and recreate it */

	while (TRUE) {
		receive();
		sleepms(200);				
		pid32 processId=create(shell, 4096, 20, "shell", 1, CONSOLE);		
		XTEST_KPRINTF("Spawning new shell with PID = %d...\n",processId);  
		resume(processId);
	}
	return OK;
    
}
