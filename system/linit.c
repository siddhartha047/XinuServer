/*	linit.c	- linit	initialize lock system */
#include <xinu.h>

/* Lab 2: Complete this function */

// declare any global variables here
struct	lockent	locktab[NLOCKS];	/* Lock table			*/

void linit(void) {
  
	// your implementation goes here
    // make sure to call this in initialize.c	
	int i=0;
	struct	lockent	*lockptr;	/* Ptr to lock table entry	*/

	for (i = 0; i < NLOCKS; i++) {
		lockptr = &locktab[i];
		lockptr->lstate = L_FREE;			
		lockptr->lqueue = newqueue();
	}
}
