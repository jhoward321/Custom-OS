#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"



//paging functions
void paging_init();
//uint32_t add_page();
//uint32_t find_empty_page();
uint32_t calc_pde_val(uint32_t processid);
void add_vidpage();
void add_page(uint32_t pde, uint32_t pd_index);
void reset_cr3();
uint32_t* get_terminal_back_page(int terminal_index);


#endif
