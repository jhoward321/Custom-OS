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

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t first_page_table[1024] __attribute__((aligned(4096)));

//enables paging
void paging_init(){

	//initialize all the pages in the directory
	uint32_t i;
	for(i = 0; i < 1024; i++)
	{
	    // This sets the following flags to the pages:
	    //   Supervisor: Only kernel-mode can access them
	    //   Write Enabled: It can be both read from and written to
	    //   Not Present: The page table is not present
	    page_directory[i] = 0x00000002;
	    first_page_table[i] = (i * 0x1000) | 3;
	}





	page_directory[0] = ((uint32_t)first_page_table) | 3;



	//the assembly below loads the page directory
	//the first instruction loads the page directory into cr3
	//the next 3 instructions set bits 4&7 of cr4 which allows pages to be
	//4MB(pse) and address translations may be shared between address spaces (PGE)
	//the finally the last 3 instructions set bit 31 of cr0 which enables paging
	asm volatile ("				  \
			movl %%eax, %%cr3	\n\
						  \
			movl %%cr4, %%eax	\n\
			orl $0x00000090, %%eax	\n\
			movl %%eax, %%cr4	\n\
						  \
			movl %%cr0, %%eax	\n\
			orl $0x80000000, %%eax	\n\
			movl %%eax, %%cr0	\n"
			:			//outputs
			:"A"(page_directory)	//inputs
			:"eax"			//clobbered registers

		);

}
