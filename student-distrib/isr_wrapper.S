/* filename: isr_wrapper.s */
.globl   ex_33
.globl   ex_40
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
