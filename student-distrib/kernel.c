/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"

/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

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
		printf ("Invalid magic number: 0x%#x\n", (unsigned) magic);
		return;
	}

	/* Set MBI to the address of the Multiboot information structure. */
	mbi = (multiboot_info_t *) addr;

	/* Print out the flags. */
	printf ("flags = 0x%#x\n", (unsigned) mbi->flags);

	/* Are mem_* valid? */
	if (CHECK_FLAG (mbi->flags, 0))
		printf ("mem_lower = %uKB, mem_upper = %uKB\n",
				(unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

	/* Is boot_device valid? */
	if (CHECK_FLAG (mbi->flags, 1))
		printf ("boot_device = 0x%#x\n", (unsigned) mbi->boot_device);

	/* Is the command line passed? */
	if (CHECK_FLAG (mbi->flags, 2))
		printf ("cmdline = %s\n", (char *) mbi->cmdline);

	if (CHECK_FLAG (mbi->flags, 3)) {
		int mod_count = 0;
		int i;
		module_t* mod = (module_t*)mbi->mods_addr;
		while(mod_count < mbi->mods_count) {
			printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
			printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
			printf("First few bytes of module:\n");
			for(i = 0; i<16; i++) {
				printf("0x%x ", *((char*)(mod->mod_start+i)));
			}
			printf("\n");
			mod_count++;
			mod++;
		}
	}
	/* Bits 4 and 5 are mutually exclusive! */
	if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
	{
		printf ("Both bits 4 and 5 are set.\n");
		return;
	}

	/* Is the section header table of ELF valid? */
	if (CHECK_FLAG (mbi->flags, 5))
	{
		elf_section_header_table_t *elf_sec = &(mbi->elf_sec);

		printf ("elf_sec: num = %u, size = 0x%#x,"
				" addr = 0x%#x, shndx = 0x%#x\n",
				(unsigned) elf_sec->num, (unsigned) elf_sec->size,
				(unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
	}

	/* Are mmap_* valid? */
	if (CHECK_FLAG (mbi->flags, 6))
	{
		memory_map_t *mmap;

		printf ("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
				(unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
		for (mmap = (memory_map_t *) mbi->mmap_addr;
				(unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
				mmap = (memory_map_t *) ((unsigned long) mmap
					+ mmap->size + sizeof (mmap->size)))
			printf (" size = 0x%x,     base_addr = 0x%#x%#x\n"
					"     type = 0x%x,  length    = 0x%#x%#x\n",
					(unsigned) mmap->size,
					(unsigned) mmap->base_addr_high,
					(unsigned) mmap->base_addr_low,
					(unsigned) mmap->type,
					(unsigned) mmap->length_high,
					(unsigned) mmap->length_low);
	}

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

	void handler0(){
		printf("Divide by zero you dumbass");
	}
	lidt(idt_desc_ptr); //load interrupt descriptor table
	//setup IDT
	//exception calls
	SET_IDT_ENTRY(idt[0], (uint32_t) handler0);
	// SET_IDT_ENTRY(idt[1], (uint32_t) handler1);
	// SET_IDT_ENTRY(idt[2], (uint32_t) handler2);
	// SET_IDT_ENTRY(idt[3], (uint32_t) handler3);
	// SET_IDT_ENTRY(idt[4], (uint32_t) handler4);
	// SET_IDT_ENTRY(idt[5], (uint32_t) handler5);
	// SET_IDT_ENTRY(idt[6], (uint32_t) handler6);
	// SET_IDT_ENTRY(idt[7], (uint32_t) handler7);
	// SET_IDT_ENTRY(idt[8], (uint32_t) handler8);
	// SET_IDT_ENTRY(idt[9], (uint32_t) handler9);
	// SET_IDT_ENTRY(idt[10], (uint32_t) handler10);
	// SET_IDT_ENTRY(idt[11], (uint32_t) handler11);
	// SET_IDT_ENTRY(idt[12], (uint32_t) handler12);
	// SET_IDT_ENTRY(idt[13], (uint32_t) handler13);
	// SET_IDT_ENTRY(idt[14], (uint32_t) handler14);
	// SET_IDT_ENTRY(idt[15], (uint32_t) handler15);
	// SET_IDT_ENTRY(idt[16], (uint32_t) handler16);
	// SET_IDT_ENTRY(idt[17], (uint32_t) handler17);
	// SET_IDT_ENTRY(idt[18], (uint32_t) handler18);
	// SET_IDT_ENTRY(idt[19], (uint32_t) handler19);
	// SET_IDT_ENTRY(idt[20], (uint32_t) handler20);
	// SET_IDT_ENTRY(idt[21], (uint32_t) handler21);
	// SET_IDT_ENTRY(idt[22], (uint32_t) handler22);
	// SET_IDT_ENTRY(idt[23], (uint32_t) handler23);
	// SET_IDT_ENTRY(idt[24], (uint32_t) handler24);
	// SET_IDT_ENTRY(idt[25], (uint32_t) handler25);
	// SET_IDT_ENTRY(idt[26], (uint32_t) handler26);
	// SET_IDT_ENTRY(idt[27], (uint32_t) handler27);
	// SET_IDT_ENTRY(idt[28], (uint32_t) handler28);
	// SET_IDT_ENTRY(idt[29], (uint32_t) handler29);
	// SET_IDT_ENTRY(idt[30], (uint32_t) handler30);
	// SET_IDT_ENTRY(idt[31], (uint32_t) handler31);
	int i;
	for(i = 0; i < 1; i++){
		idt[i].seg_selector = KERNEL_CS;
		idt[i].reserved4 = 0x00;
		idt[i].reserved3 = 0;
		idt[i].reserved2 = 1;
		idt[i].reserved1 = 1;
		idt[i].size = 1;//side is D, 1 = 32 bits
		idt[i].reserved0 = 0;
		idt[i].dpl = 0;
		idt[i].present = 1;
	}
	//set system call
	//idt[128].seg_selector = 
	i /= 0;

	/* Init the PIC */
	i8259_init();

	/* Initialize devices, memory, filesystem, enable device interrupts on the
	 * PIC, any other initialization stuff... */

	/* Enable interrupts */
	/* Do not enable the following until after you have set up your
	 * IDT correctly otherwise QEMU will triple fault and simple close
	 * without showing you any output */
	/*printf("Enabling Interrupts\n");
	sti();*/

	/* Execute the first program (`shell') ... */

	/* Spin (nicely, so we don't chew up cycles) */
	asm volatile(".1: hlt; jmp .1;");
}

