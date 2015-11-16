#include "paging.h"

//references: 	http://wiki.osdev.org/Setting_Up_Paging
//		http://wiki.osdev.org/Paging
//		starting at page 3-21 in IA

//linear address for 4KB page
//31       22|21       12|11      0
//directory  |table      |offset to physical address

//linear address for 4MB page
//31       22|21                 0
//directory              |offset to physical address


//magic numbers
#define NUM_INDEXES 1024 				//number of indexes in directory
#define ALIGN_SIZE 4096 				//4k, size for alignment
#define NOT_PRESENT 0x00000002 			//key for mapping kernel as not present
#define PRESENT 0x00000003 				//key for mapping kernel as present, and read/write
#define KERNEL_VIRTADR 0x400000 		//kernel virtual address (4MB)
#define VID_MEM_LOC 0xB8 				//video memory location
#define PAGE_DIREC_SIZE_MASK 0x80 		//Mask to set Page Directory Size: stores the page size for that specific entry. (4MB)
#define FOUR_MB 0x0400000
#define USERBIT 0x4

uint32_t page_directory[NUM_INDEXES] __attribute__((aligned(ALIGN_SIZE)));
uint32_t first_page_table[NUM_INDEXES] __attribute__((aligned(ALIGN_SIZE)));

//enables paging
void paging_init(){

	//initialize the directory to empty
	uint32_t i;
	for(i = 0; i < NUM_INDEXES; i++)
	{
	    // This sets the following flags to the pages:
	    //   Supervisor: Only kernel-mode can access them
	    //   Write Enabled: It can be both read from and written to
	    //   Not Present: The page table is not present
	    page_directory[i] = NOT_PRESENT;

	    //first_page_table[i] = (i * 0x1000) | 3; //x1000 is just 4096
	}


	first_page_table[VID_MEM_LOC] = (VID_MEM_LOC * ALIGN_SIZE) | PRESENT; //map memory 0mb to 4mb to the table
	page_directory[0] = ((uint32_t)first_page_table) | PRESENT;
	//directory 1 is kernel
	page_directory[1] = KERNEL_VIRTADR | (PAGE_DIREC_SIZE_MASK | PRESENT); //map kernel as present


	//the assembly below loads the page directory
	//the first instruction loads the page directory into cr3
	//the next 3 instructions set bits 4&7 of cr4 which allows pages to be
	//4MB(pse) and address translations may be shared between address spaces (PGE)
	//the finally the last 3 instructions set bit 31 of cr0 which enables paging

	//the first or might need to change the 9 to a 1
	asm volatile (
			"movl %0, %%eax \n\
			movl %%eax, %%cr3	\n\
			movl %%cr4, %%eax	\n\
			orl $0x00000010, %%eax	\n\
			movl %%eax, %%cr4	\n\
			movl %%cr0, %%eax	\n\
			orl $0x80000000, %%eax	\n\
			movl %%eax, %%cr0"
			:			//outputs
			:"g"(page_directory)	//inputs
			:"eax"			//clobbered registers

		);

}
//find empty page directory entry index
// uint32_t find_empty_page(){
// 	uint32_t i = 2;
// 	while(page_directory[i++] & PRESENT);
// 	return i;
// }
// //
void add_page(uint32_t pde, uint32_t pd_index){
	page_directory[pd_index] = pde;
}
uint32_t calc_pde_val(uint32_t processid){
	uint32_t pde = (FOUR_MB + processid * FOUR_MB) | USERBIT | PAGE_DIREC_SIZE_MASK | PRESENT;
	return pde;
}

void reset_cr3(){
	asm volatile (
		"movl %0, %%cr3"
		:
		:"c"(page_directory)
	);
}
