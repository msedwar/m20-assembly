; ==============================================================================
; Test file 4
;   Prints numbers in a for-loop
;
;   Author:         Matthew Edwards
;   Dependencies:   print, string
; ==============================================================================

; EXPORTS ======================================================================

entry main


; IMPORTS ======================================================================

extern itoua
extern memcpy
extern memset
extern print
extern strcpy


; TEXT =========================================================================

section .text

main:
    push lp

    bwl init        ; init()

    pop lp
    mov r0, #0
    mov pc, lp      ; return 0

init:
    push lp

    sub sp, sp, #86 ; r0 = alloc(82)
    add r4, sp, #45

    mov r0, r4
    mov r3, #0
    strb r3, r4, #40
    ldrb r1, char
    mov r2, #40
    bwl memset      ; memset(buf, char, #40)
    bwl print       ; print(buf)

    mov r2, #41
    mov r1, r4
    add r5, sp, #4
    mov r0, r5
    bwl memcpy      ; memcpy(buf2, buf, #41)
    bwl print       ; print(buf2)

    mov r0, r5
    mov r1, 0x35
    mov r2, #20
    bwl memset      ; memset(buf2, 0x35, #20)
    bwl print       ; print(buf2)

    mov r0, r4
    mov r1, r5
    bwl strcpy      ; strcpy(buf, buf2)
    bwl print       ; print(buf)

    add sp, sp, #86 ; free(82)

    pop lp
    mov pc, lp      ; return


; DATA =========================================================================

section .data

char:
    db "A"
