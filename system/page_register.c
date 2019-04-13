#include<xinu.h>

unsigned long tmp;

unsigned long read_cr0(void);
void set_cr0(unsigned long n);
void set_cr3(unsigned long n);
unsigned long read_cr3(void);
unsigned long read_cr2(void);
void enable_paging(void);
void set_page_directory(unsigned long n);


unsigned long read_cr0(void){
  intmask mask;
  mask = disable();
  unsigned long local_tmp;

  asm("pushl %eax");
  asm("movl %cr0, %eax");
  asm("movl %eax, tmp");
  asm("popl %eax");

  local_tmp = tmp;
  restore(mask);

  return local_tmp;
}

void set_cr0(unsigned long n) {
  intmask mask;
  mask = disable();

  tmp = n;
  asm("pushl %eax");
  asm("movl tmp, %eax");   
  asm("movl %eax, %cr0");
  asm("popl %eax");

  restore(mask);
  return;
}

void enable_paging(void){
	intmask mask=disable();
	unsigned long n=read_cr0();
	n=n|(0x80000000);
	set_cr0(n);	
	restore(mask);
	return;
}

void set_page_directory(unsigned long n){
	intmask mask=disable();
	n=n<<12;
	set_cr3(n);
	restore(mask);
}

void set_cr3(unsigned long n) {
  /* we cannot move anything directly into
     %cr4. This must be done via %eax. Therefore
     we save %eax by pushing it then pop
     it at the end.
  */

  intmask mask=disable();

  tmp = n;
  asm("pushl %eax");
  asm("movl tmp, %eax");
  asm("movl %eax, %cr3");
  asm("popl %eax");

  restore(mask);
  return;
}


unsigned long read_cr3(void) {

  intmask mask = disable();
  unsigned long local_tmp;

  asm("pushl %eax");
  asm("movl %cr3, %eax");
  asm("movl %eax, tmp");
  asm("popl %eax");

  local_tmp = tmp;

  restore(mask);

  return local_tmp;
}


unsigned long read_cr2(void) {

  intmask mask;
  mask = disable();
  unsigned long local_tmp;

  asm("pushl %eax");
  asm("movl %cr2, %eax");
  asm("movl %eax, tmp");
  asm("popl %eax");

  local_tmp = tmp;

  restore(mask);

  return local_tmp;
}