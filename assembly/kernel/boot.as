; ==============================================================================
; Operating System Bootloader
;
;   Author:         Matthew Edwards
;   Dependencies:   reset_handler
; ==============================================================================

extern __reset

entry __interrupt_vector_table


; Interrupt Vector Table -------------------------------------------------------

section .text

__interrupt_vector_table:
    ldr pc, pc, 0x3c            ; <__reset>
    ldr pc, pc, 0x3c            ; <__undefined_instruction>
    ldr pc, pc, 0x3c            ; <__software_interrupt>
    ldr pc, pc, 0x3c            ; <__prefetch_abort>
    ldr pc, pc, 0x3c            ; <__data_abort>
    ldr pc, pc, 0x3c            ; <__usage_abort>
    ldr pc, pc, 0x3c            ; reserved
    ldr pc, pc, 0x3c            ; reserved
    ldr pc, pc, 0x3c            ; reserved
    ldr pc, pc, 0x3c            ; reserved
    ldr pc, pc, 0x3c            ; reserved
    ldr pc, pc, 0x3c            ; reserved
    ldr pc, pc, 0x3c            ; reserved
    ldr pc, pc, 0x3c            ; reserved
    ldr pc, pc, 0x3c            ; reserved
    ldr pc, pc, 0x3c            ; <__irq>

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

__undefined_instruction:
__software_interrupt:
__prefetch_abort:
__data_abort:
__usage_abort:
__irq:
    mov r0, 0x7F
    halt
