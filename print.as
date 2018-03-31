; ==============================================================================
; Printing Utilities Library File
;
;   Author:         Matthew Edwards
;   Dependencies:   string
; ==============================================================================

; EXPORTS ======================================================================

global print


; IMPORTS ======================================================================

extern strlen


; DEFINITIONS ==================================================================

section .text

; ------------------------------------------------------------------------------
;   void print( void * buffer )
;   Prints a C-string to stdout
;   r0          : void * buffer, Pointer to C-string
print:
    push lp             ; Save callee-save registers
    push r7

    mov r2, #-1         ; Perform custom strlen operation
    print_strlen_loop:  ; (prevents multiple register saves and swaps)
    add r2, r2, #1
    ldrb r3, r0, r2
    cmp r3, #0
    bne print_strlen_loop

    mov r1, r0
    mov r0, #1
    mov r7, #3          ; SWI_write
    swi #0              ; write(stdout, buffer, len)

    pop r7              ; Load callee-save registers
    pop lp
    mov pc, lp          ; return
