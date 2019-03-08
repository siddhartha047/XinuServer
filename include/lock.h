/*  lock.h	*/


// declare variables, #defines, 
#ifndef READ 
#define READ 0
#define READL READ
#endif
#ifndef WRITE 
#define WRITE 1
#define WRITEL WRITE
#endif
#ifndef DELETED
#define DELETED (-4)
#endif

#define	L_FREE	0		/* Lock table entry is available	*/
#define	L_USED	1		/* Lock table entry is in use	*/

#define LPR_WAIT 1		
#define LPR_FREE 0


struct	lockent {
	//struct members
		byte	lstate;		/* Whether entry is S_FREE or S_USED	*/
		
		int32	rwait;		/* Count how many waiting*/		
		int32 	wwait;		

		int32 	rcount;		/* How many reader lock holding */
		int32 	wcount;		/* is writer lock holding or not*/

		qid16	lqueue;		/* Queue of processes that are waiting	*/
		int 	timestamp; //track lock delete or creation time
				
		byte	lmode[NPROC];	//mode of waiting process READ/WRITE
		byte 	wprocess[NPROC];

		int 	maxprio;
};

extern	struct	lockent locktab[];

#define	isbadlock(s)	((int32)(s) < 0 || (s) >= NLOCKS)



/* Lab 3 lock table */

//extern struct lockent locktab[NLOCKS];
