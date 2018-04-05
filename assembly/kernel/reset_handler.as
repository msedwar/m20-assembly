; ==============================================================================
; Operating System Bootloader
;
;   Author:         Matthew Edwards
;   Dependencies:   io
; ==============================================================================

; IMPORTS ----------------------------------------------------------------------

extern puts


; EXPORTS ----------------------------------------------------------------------

global __reset


; Reset Handler ----------------------------------------------------------------

section .text

__reset:
    mov r4, #21

    __reset_newline_loop:
    mov r0, __reset_newline
    bwl puts                    ; puts(&__reset_newline)
    sub r4, r4, #1
    cmp r4, #0
    bge __reset_newline_loop

    mov r0, __reset_str
    bwl puts                    ; puts(&__reset_str)

    swi #0

    mov r0, #0
    halt


; Reset Handler Static Data ----------------------------------------------------

section .data

__reset_newline:
    db "\n"
    db 0x00
__reset_str:
    db "Kernel (Basic v1 by Matthew Edwards)\nResetting...\n"
    db 0x00
