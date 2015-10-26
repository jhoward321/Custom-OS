/* filename: isr_wrapper.s */
.globl   ex_33
.globl   ex_40
.align   4

ex_33:
        pushl %EAX
        pushl %ECX
        pushl %EDX
        pushl %EBX
        pushl %ESP
        pushl %EBP
        pushl %ESI
        pushl %EDI

        cld
        call keyboard_handler

        popl %EDI
        popl %ESI
        popl %EBP
        popl %ESP
        popl %EBX
        popl %EDX
        popl %ECX
        popl %EAX
        iret

ex_40:
        pushl %EAX
        pushl %ECX
        pushl %EDX
        pushl %EBX
        pushl %ESP
        pushl %EBP
        pushl %ESI
        pushl %EDI

        cld
        call rtc_handler

        popl %EDI
        popl %ESI
        popl %EBP
        popl %ESP
        popl %EBX
        popl %EDX
        popl %ECX
        popl %EAX
        iret
