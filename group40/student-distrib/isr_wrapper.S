/* filename: isr_wrapper.s */
.globl   ex_33
.globl   ex_40
.globl	 ex_128
.align   4

ex_33:
        pusha
        cld
        call keyboard_handler
        popa
        iret

ex_40:
        pusha
        cld
        call rtc_handler
        popa
        iret

ex_128:
        pusha			#save all regs
	pushl %edx		#push third arg
        pushl %ecx		#push second arg
        pushl %ebx		#push first arg
#        cld
	cmpl $10, %ecx		#compare to 10, the max number of sys calls
	ja ret_error		#ret error when sys call is greater than 10
        call *jumptable(,%eax,4)#call handler
        movl %eax, ret_val	#handler's return val is in eax save in ret_val
        popa			#restore all regs
        movl ret_val, %eax	#restore ret_val to eax
        iret

ret_error:
	popa			#restore all regs
	movl $-1, %eax		#restore ret val to eax
	iret

ret_val:			#temporarily stores the ret val
        .long 0x0
jumptable:
	.long sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap, sys_set_handler, sys_sigreturn
