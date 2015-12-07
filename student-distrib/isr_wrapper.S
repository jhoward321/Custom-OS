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
	pushal
	pushl %edx
	pushl %ecx
	pushl %ebx

	cmpl $0, %eax		#compare to 0, no sys call 0
	je ret_error		#ret error when sys call is greater than 10

	cmpl $10, %eax		#compare to 10, the max number of sys calls
	ja ret_error		#ret error when sys call is greater than 10

	call *jumptable(,%eax,4)#call handler
	movl %eax, ret_save
	addl $12, %esp
	popal
	movl ret_save, %eax
	iret


ret_error:
	addl $12, %esp
	popal
	movl $-1, %eax		#restore ret val to eax
	iret

ret_save:
	.long 0x0

jumptable:
	.long 0x0, sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap, sys_set_handler, sys_sigreturn
