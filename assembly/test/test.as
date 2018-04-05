; ==============================================================================
; Test file
; ==============================================================================

global _main
extern printStr

; ==============================================================================
; Text
; ==============================================================================
section .text


; Main Function ----------------------------------------------------------------
_main:
    mov r3, string              ; Load string ptr
    mov r4, str_end
    sub r4, r4, r3              ; Calculate string length

    _main_return:               ; Fragment: Return
    mov r0, #1
    mov r1, #0
    swi 0xF0                    ; syscall return code 0
    mov pc, lp                  ; Return _main function

    ldr r0, r1, test

test:
    str r2, test
    bal #1
    bwl r4
    swi 0x7FFFFF

    ldr r1, 0xAAAA
    ldr r1, r2
    ldr r1, r2, 0xBBB
    ldr r1, r2, r3

    str r1, test
    halt


; ==============================================================================
; Data
; ==============================================================================
section .data

string:
    db "Hello \"world!"
    db 0x0A
str_end: 0x00

data:
    dw _main
    0x00
    0xFF
    0x00, 0xFF
    0xABCDEF
    #16
    dw #-1
    0b01010101
    dw $
    dh 0xABCD
    dd 0xABC
    dd 0x0123456789ABCDEF
    space #16
    db 0123
