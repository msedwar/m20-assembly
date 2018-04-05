; ==============================================================================
; Test file 4
;   Prints numbers in a for-loop
;
;   Author:         Matthew Edwards
;   Dependencies:   stdio, string
; ==============================================================================

; EXPORTS ======================================================================

entry main


; IMPORTS ======================================================================

; string -------------------------------
extern itoa
extern memcpy
extern memset
extern strcat
extern strcpy
extern strncpy


; stdio --------------------------------
extern puts


; stdlib -------------------------------
extern salloc
extern sfree


; TEXT =========================================================================

section .text

main:
    push lp

    bwl init            ; init()

    mov r0, #20
    bwl salloc          ; buf = salloc(#20)
    mov r4, r0

    mov r0, _num
    ldr r0, r0
    mov r1, r4
    mov r2, #16
    bwl itoa            ; itoa(#127, buf, #10)

    mov r0, r4
    bwl puts            ; puts(buf)

    bwl sfree           ; sfree(buf)

    pop lp
    mov r0, #0
    mov pc, lp          ; return 0

init:
    push lp

    sub sp, sp, #150    ; r0 = alloc(150)
    add r4, sp, #100
    add r5, sp, #50
    add r6, sp, #0

    mov r0, r4
    mov r3, #0
    strb r3, r4, #40
    ldrb r1, char
    mov r2, #40
    bwl memset          ; memset(buf, char, #40)
    bwl puts            ; puts(buf)

    mov r2, #41
    mov r1, r4
    mov r0, r5
    bwl memcpy          ; memcpy(buf2, buf, #41)
    bwl puts            ; puts(buf2)

    mov r0, r5
    mov r1, 0x35
    mov r2, #20
    bwl memset          ; memset(buf2, 0x35, #20)
    bwl puts            ; puts(buf2)

    mov r0, r4
    mov r1, r5
    bwl strcpy          ; strcpy(buf, buf2)
    bwl puts            ; puts(buf)

    mov r0, r5
    mov r1, 0x61
    mov r2, #30
    mov r3, #0
    strb r3, r0, #31
    bwl memset          ; memset(buf2, 0x61, #30)
    bwl puts            ; puts(buf2)

    mov r0, r6
    mov r1, r5
    mov r2, #50
    mov r3, #0
    bwl strncpy         ; strncpy(buf3, buf2, #50)
    bwl puts            ; puts(buf3)

    mov r0, r4
    mov r1, r5
    mov r2, #30
    mov r3, #0
    strb r3, r0, #30
    bwl strncpy         ; strncpy(buf, buf2, #30)
    bwl puts            ; puts(buf)

    mov r0, r6
    mov r1, r4
    bwl strcat          ; strcat(buf3, buf)
    bwl puts            ; puts(buf3)

    add sp, sp, #150    ; free(150)

    pop lp
    mov pc, lp          ; return


; DATA =========================================================================

section .data

char:
    db "A"
_num:
    dw 0xFFFF
