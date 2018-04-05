; ==============================================================================
; Operating System Bootloader
;
;   Author:         Matthew Edwards
;   Dependencies:   
; ==============================================================================

entry __interrupt_vector_table


; Interrupt Vector Table -------------------------------------------------------

section .text

__interrupt_vector_table:
    ldr pc, pc, 0x40            ; <__reset>
    ldr pc, pc, 0x40            ; <__undefined_instruction>
    ldr pc, pc, 0x40            ; <__software_interrupt>
    ldr pc, pc, 0x40            ; <__prefetch_abort>
    ldr pc, pc, 0x40            ; <__data_abort>
    ldr pc, pc, 0x40            ; <__usage_abort>
    ldr pc, pc, 0x40            ; reserved
    ldr pc, pc, 0x40            ; reserved
    ldr pc, pc, 0x40            ; reserved
    ldr pc, pc, 0x40            ; reserved
    ldr pc, pc, 0x40            ; reserved
    ldr pc, pc, 0x40            ; reserved
    ldr pc, pc, 0x40            ; reserved
    ldr pc, pc, 0x40            ; reserved
    ldr pc, pc, 0x40            ; reserved
    ldr pc, pc, 0x40            ; <__irq>

    dw __reset                  ; <__reset>
    dw __undefined_instruction  ; <__undefined_instruction>
    dw __software_interrupt     ; <__software_interrupt>
    dw __prefetch_abort         ; <__prefetch_abort>
    dw __data_abort             ; <__data_abort>
    dw __usage_abort            ; <__usage_abort>
    dw 0x00                     ; reserved
    dw 0x00                     ; reserved
    dw 0x00                     ; reserved
    dw 0x00                     ; reserved
    dw 0x00                     ; reserved
    dw 0x00                     ; reserved
    dw 0x00                     ; reserved
    dw 0x00                     ; reserved
    dw 0x00                     ; reserved
    dw __irq                    ; <__irq>

; Label Definitions to allow compilation

__reset:
__undefined_instruction:
__software_interrupt:
__prefetch_abort:
__data_abort:
__usage_abort:
__irq:
    halt

