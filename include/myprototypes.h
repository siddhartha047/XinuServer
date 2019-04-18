/* myprototypes.h */

#ifndef __MYPROTOTYPES_H_
#define __MYPROTOTYPES_H_

//sid: added my meh


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


typedef struct
{
	uint32 pg_offset : 12;
	uint32 pt_offset : 10;
	uint32 pd_offset : 10;
}vd_t;

//
#define GLOBAL_PAGE_NO 4
//sid: 4 global, 1 for device, 1 for page directory
extern pt_t *global_pt[GLOBAL_PAGE_NO];
extern pt_t *device_pt; //global page directory
extern pd_t *global_pd; //global page directory


//sid: base references for some address
#define VPN0		4096
#define DEVICE_VPN 589824  //Starting virtual page number
#define DEVICE_PTN 576     //Device Page Index

//frame states
#define FRAME_USED 1 //frame is used
#define FRAME_FREE 0 //frame is free

//frame types
#define FRAME_NONE -1 //not defined yet
#define FRAME_PD 0 //farme page directory
#define FRAME_PT 1 //frame page table
#define FRAME_PR 2 //rescident page

//frame dirtiness
#define FRAME_DIRTY 1
#define FRAME_NOT_DIRTY 0

//sid: macros for different address transformation
#define frameno_to_address(frameNo) (uint32)(((uint32)frameNo+FRAME0)*NBPG)
#define address_to_vpn(address) (uint32)((uint32)address/NBPG)
#define vpn_to_address(vpn) (uint32)((uint32)vpn*NBPG)
#define address_to_frameno(address) (uint32)((uint32)address/NBPG-FRAME0)
#define frameno_to_vpn(frameNo) (uint32)((uint32)frameNo+FRAME0)


extern sid32 fault_sem;

//--------------------------------//
//sid: in file mmu_frame.c
extern int32 initialize_frame(void);
extern int32 get_one_frame(void);
extern int32 find_free_frame(void);


//--------------------------------//
//sid: mmu_page_swapping.c
extern int32 swap_frame_back(int32 frameNo);



//--------------------------------//
//sid: in file mmu_page.c
extern int32 initialize_global_page_table(void);
extern pd_t* get_page_directory(void);
extern pt_t *get_one_page(void);

//--------------------------------//
//sid: in assembly file pagefault.s
extern void pagefault(void);

//--------------------------------//
//sid: in file page_fault_handler.c
extern void page_fault_handler(void);


//--------------------------------//
//sid: in file page_register.c
extern unsigned long tmp;
extern unsigned long read_cr0(void);
extern void set_cr0(unsigned long n);
extern void set_cr3(unsigned long n);
extern unsigned long read_cr3(void);
extern unsigned long read_cr2(void);
extern void enable_paging(void);
extern void set_page_directory(unsigned long n);

//--------------------------------//
//sid: for bs mapping
typedef struct{
	pid32 pid; //process id
	uint32 vpn; //starting page number
	uint32 npages;
	bsd_t bsid;
	int32 allocated;
}backing_store_map;

extern backing_store_map backing_store_map_tab[];

//--------------------------------//

//sid: in file mmu_backing_store.c
extern void initialize_backing_store(void);
extern int32 add_bs_map(pid32 pid, uint32 vpn, uint32 npages, bsd_t bsid);
extern int32 remove_bs_map(pid32 pid);
extern backing_store_map* get_bs_map(pid32 pid, uint32 vpn);


extern int32 fault_counts;

//--------------------------------//
//in file mmu_fifo_policy.c
extern int32 get_frame_fifo(void);
extern void addToFrameList(frame_t *frame_entry);
extern int32 remove_frame_fifo(int32 frameNo);
extern void printFrameList(frame_t *frame_entry);

//--------------------------------//
//in file mmu_gca_policy.c
extern int32 get_frame_gca(void);

extern int32 lframeNo; //this is for global clock
#define SWEEP_TIMES 3

#endif // __MYPROTOTYPES_H_