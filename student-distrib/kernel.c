/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "exceptions.h"
#include "rtc.h"
#include "keyboard.h"
#include "paging.h"
#include "fs.h"

/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

//TEMPORARY
#define FILE_NAME_STRING_LEN 33

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void
entry (unsigned long magic, unsigned long addr)
{
	multiboot_info_t *mbi;


	/* Clear the screen. */
	clear();

	/* Am I booted by a Multiboot-compliant boot loader? */
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		//printf ("Invalid magic number: 0x%#x\n", (unsigned) magic);
		return;
	}

	/* Set MBI to the address of the Multiboot information structure. */
	mbi = (multiboot_info_t *) addr;

	/* Print out the flags. */
	//printf ("flags = 0x%#x\n", (unsigned) mbi->flags);

	/* Are mem_* valid? */
	// if (CHECK_FLAG (mbi->flags, 0))
		//printf ("mem_lower = %uKB, mem_upper = %uKB\n",
				// (unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

	// /* Is boot_device valid? */
	// if (CHECK_FLAG (mbi->flags, 1))
	// 	//printf ("boot_device = 0x%#x\n", (unsigned) mbi->boot_device);

	// /* Is the command line passed? */
	// if (CHECK_FLAG (mbi->flags, 2))
	// 	//printf ("cmdline = %s\n", (char *) mbi->cmdline);


	if (CHECK_FLAG (mbi->flags, 3)) {
		int mod_count = 0;
		int i;
		module_t* mod = (module_t*)mbi->mods_addr;

		boot_block = (boot_block_t*) mod->mod_start;

		while(mod_count < mbi->mods_count) {
			//printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
			//printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
			//printf("First few bytes of module:\n");
			for(i = 0; i<16; i++) {
				//printf("0x%x ", *((char*)(mod->mod_start+i)));
			}
			//printf("\n");
			mod_count++;
			mod++;
		}
	}
	/* Bits 4 and 5 are mutually exclusive! */
	if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
	{
		//printf ("Both bits 4 and 5 are set.\n");
		return;
	}

	/* Is the section header table of ELF valid? */
	if (CHECK_FLAG (mbi->flags, 5))
	{
		elf_section_header_table_t *elf_sec = &(mbi->elf_sec);

		//printf ("elf_sec: num = %u, size = 0x%#x,"
				// " addr = 0x%#x, shndx = 0x%#x\n",
				// (unsigned) elf_sec->num, (unsigned) elf_sec->size,
				// (unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
	}

	// /* Are mmap_* valid? */
	// if (CHECK_FLAG (mbi->flags, 6))
	// {
	// 	memory_map_t *mmap;

	// 	printf ("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
	// 			(unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
	// 	for (mmap = (memory_map_t *) mbi->mmap_addr;
	// 			(unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
	// 			mmap = (memory_map_t *) ((unsigned long) mmap
	// 				+ mmap->size + sizeof (mmap->size)))
	// 		printf (" size = 0x%x,     base_addr = 0x%#x%#x\n"
	// 				"     type = 0x%x,  length    = 0x%#x%#x\n",
	// 				(unsigned) mmap->size,
	// 				(unsigned) mmap->base_addr_high,
	// 				(unsigned) mmap->base_addr_low,
	// 				(unsigned) mmap->type,
	// 				(unsigned) mmap->length_high,
	// 				(unsigned) mmap->length_low);
	// }

	/* Construct an LDT entry in the GDT */
	{
		seg_desc_t the_ldt_desc;
		the_ldt_desc.granularity    = 0;
		the_ldt_desc.opsize         = 1;
		the_ldt_desc.reserved       = 0;
		the_ldt_desc.avail          = 0;
		the_ldt_desc.present        = 1;
		the_ldt_desc.dpl            = 0x0;
		the_ldt_desc.sys            = 0;
		the_ldt_desc.type           = 0x2;

		SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
		ldt_desc_ptr = the_ldt_desc;
		lldt(KERNEL_LDT);
	}

	/* Construct a TSS entry in the GDT */
	{
		seg_desc_t the_tss_desc;
		the_tss_desc.granularity    = 0;
		the_tss_desc.opsize         = 0;
		the_tss_desc.reserved       = 0;
		the_tss_desc.avail          = 0;
		the_tss_desc.seg_lim_19_16  = TSS_SIZE & 0x000F0000;
		the_tss_desc.present        = 1;
		the_tss_desc.dpl            = 0x0;
		the_tss_desc.sys            = 0;
		the_tss_desc.type           = 0x9;
		the_tss_desc.seg_lim_15_00  = TSS_SIZE & 0x0000FFFF;

		SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

		tss_desc_ptr = the_tss_desc;

		tss.ldt_segment_selector = KERNEL_LDT;
		tss.ss0 = KERNEL_DS;
		tss.esp0 = 0x800000;
		ltr(KERNEL_TSS);
	}

	// clear();			//clear the screen

	set_exeptions();		//set up known exceptions in table
	paging_init();			//enable paging

	lidt(idt_desc_ptr); 		//load interrupt descriptor table


	/* Init the PIC */
	i8259_init();
	//unmask needed irq lines
	enable_irq(KEYBOARD_IRQ);			//enable keyboard
	enable_irq(SLAVE_IRQ);			//enable slave irq
	enable_irq(RTC_IRQ);			//enable RTC
	/* Initialize devices, memory, filesystem, enable device interrupts on the
	 * PIC, any other initialization stuff... */

	//init RTC
	rtc_init();

	/* Enable interrupts */
	/* Do not enable the following until after you have set up your
	 * IDT correctly otherwise QEMU will triple fault and simple close
	 * without showing you any output */
	sti();


	//printf("Enabling Interrupts\n");

	//disable_irq(RTC_IRQ); 				//comment out for test RTC video mem

	//int32_t i = 0;
	//uint32_t* ptr = NULL;
	//i = *ptr;
	//disable_irq(RTC_IRQ); 				//comment out for test RTC video mem


//also for testing exceptions with variable i:
	// uint32_t i = 0;
	//uint32_t* ptr = NULL;
	//i = *ptr;			//use this line to test page fault
	// i++;
	//i= 1/0;			//use this line to test divide by zero


	//=========START FILE SYSTEM TEST CODE=========
	
	// uint8_t buf[10000];
	// uint32_t i;
	// uint8_t file_name[FILE_NAME_STRING_LEN] = "frame0.txt";
	
	// for(i=0; i<MAX_FILE_NAME_LENGTH; i++)
	// 	printf("%c",file_name[i]);

	// printf("\n");
	// dentry_t temp;
	// uint32_t read_count;

	// read_dentry_by_name(file_name, &temp);


	// // uint32_t retval = read_dentry_by_name(file_name, &temp);
	// // printf("Retval = %d\n",retval);

	// uint32_t* curr_inode_address = ((uint32_t*)((uint32_t)boot_block + BYTES_PER_BLOCK * (temp.inode_number + 1)));
	// uint32_t file_length = *curr_inode_address;
	// read_count = read_data(temp.inode_number, 0, buf, file_length);
	
	// // printf("File Length = %d\n",file_length);
	// // printf("Read count = %d\n",read_count);
	// // printf("Inode number = %d\n", temp.inode_number);
	// // printf("%s\n",file_name);
	// printf("File length = %d\n",file_length);

	// for (i=0; i<file_length; i++){
	// 	printf("%c",buf[i]);
	// }


	//==========END FILE SYSTEM TEST CODE==========		





	/* Execute the first program (`shell') ... */
	keyboard_init();

	while(1){
	}

	/* Spin (nicely, so we don't chew up cycles) */
	asm volatile(".1: hlt; jmp .1;");
}
