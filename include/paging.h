/* paging.h */

#ifndef __PAGING_H_
#define __PAGING_H_

/* Structure for a page directory entry */

typedef struct {
	unsigned int pd_pres	: 1;		/* page table present?		*/
	unsigned int pd_write : 1;		/* page is writable?		*/
	unsigned int pd_user	: 1;		/* is use level protection?	*/
	unsigned int pd_pwt	: 1;		/* write through cachine for pt? */
	unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
	unsigned int pd_acc	: 1;		/* page table was accessed?	*/
	unsigned int pd_mbz	: 1;		/* must be zero			*/
	unsigned int pd_fmb	: 1;		/* four MB pages?		*/
	unsigned int pd_global: 1;		/* global (ignored)		*/
	unsigned int pd_avail : 3;		/* for programmer's use		*/
	unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {
	unsigned int pt_pres	: 1;		/* page is present?		*/
	unsigned int pt_write : 1;		/* page is writable?		*/
	unsigned int pt_user	: 1;		/* is use level protection?	*/
	unsigned int pt_pwt	: 1;		/* write through for this page? */
	unsigned int pt_pcd	: 1;		/* cache disable for this page? */
	unsigned int pt_acc	: 1;		/* page was accessed?		*/
	unsigned int pt_dirty : 1;		/* page was written?		*/
	unsigned int pt_mbz	: 1;		/* must be zero			*/
	unsigned int pt_global: 1;		/* should be zero in 586	*/
	unsigned int pt_avail : 3;		/* for programmer's use		*/
	unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;

#define PAGEDIRSIZE	1024
#define PAGETABSIZE	1024

#define NBPG		4096	/* number of bytes per page	*/
#define FRAME0		1024	/* zero-th frame		*/

#define DEVICE_VPN 589824  //Starting virtual page number
#define DEVICE_PTN 576     //Device Page Index

#ifndef NFRAMES
#define NFRAMES		3072	/* number of frames		*/
#endif

#define MAP_SHARED 1
#define MAP_PRIVATE 2

#define FIFO 3
#define GCA 4

#define MAX_ID		7		/* You get 8 mappings, 0 - 7 */
#define MIN_ID		0

extern int32	currpolicy;


///Added by @sid:------------------------------/////

//frame types
#define FRAME_USED 1 //frame is used
#define FRAME_FREE 0 //frame is free

#define FRAME_NONE -1 //not defined yet
#define FRAME_PD 0 //farme page directory
#define FRAME_PT 1 //frame page table
#define FRAME_PR 2 //rescident page

#define FRAME_DIRTY 1
#define FRAME_NOT_DIRTY 0


//sid: macros for different address transformation
#define frameno_to_address(frameNo) (uint32)(((uint32)frameNo+FRAME0)*NBPG)
#define address_to_vpn(address) (uint32)((uint32)address/NBPG)
//structure for frame
typedef struct frame
{
	int32 id; //frame no
	int32 type; //rescident, pd or pt
	int32 state; //frame occupied or free	
	int32 dirty; //changed
	struct frame *next; //points to next frame	
}frame_t;

extern frame_t frame_tab[];
extern frame_t *frame_head;

typedef struct 
{
	int32 pid; //which process currently holds it
	uint32 vpn; //vpn of the that process
	int32 refcount; //how many times this frame has been reffered
}inverted_page_t;

extern inverted_page_t inverted_page_tab[];


//sid: in file mmu_frame.c
extern int32 initialize_frame(void);
extern int32 get_one_frame(void);


#define GLOBAL_PAGE_NO 4

//sid: 4 global, 1 for device, 1 for page directory
extern pt_t *global_pt[GLOBAL_PAGE_NO];
extern pt_t *device_pt; //global page directory
extern pd_t *global_pd; //global page directory

//sid: in file mmu_page.c
extern int32 initialize_global_page_table(void);
extern pd_t* get_page_directory(void);


//sid: in assembly file pagefault.s
extern void pagefault(void);

//sid: in file page_fault_handler.c
extern void page_fault_handler(void);

//sid: in file page_register.c
extern void enable_paging(void);
extern void set_page_directory(unsigned long pd);


#endif // __PAGING_H_