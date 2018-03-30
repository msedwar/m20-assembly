; ==============================================================================
; Test library file (Printing utilities)
; ==============================================================================

global print
global strlen
global itoua

; DEFINITIONS ==================================================================
section .text

; ------------------------------------------------------------------------------
;   void itoua(uint, void*)
;   Transforms an unsigned integer into an ascii string (base 10)
;   r0          : uint value, Integer to transform into string
;   r1          : void* buffer, Pointer to buffer
itoua:
    cmp r0, #0
    mov r3, r1
    mov r2, #0
    strb r2, r1         ; Add null-terminator
    add r1, r1, #1      ; buffer++
    beq itoua_put_zero

    itoua_loop:
    udv r2, r0, #10
    mul r2, r2, #10
    sub r2, r0, r2      ; temp = value % 10
    udv r0, r0, #10     ; value /= 10
    add r2, r2, 0x30    ; temp = temp + '0'
    strb r2, r1         ; *buffer = temp
    add r1, r1, #1      ; buffer++
    itoua_loop_cond:
    cmp r0, #0
    bne itoua_loop      ; while (value != 0)

    itoua_return:
    sub r1, r1, #1

    itoua_reverse:
    ldrb r2, r3
    ldrb r0, r1
    strb r2, r1
    strb r0, r3
    add r3, r3, #1
    sub r1, r1, #1
    cmp r1, r3
    bgt itoua_reverse

    mov pc, lp          ; return

    itoua_put_zero:
    mov r2, 0x30        ; temp = '0'
    str r2, r1          ; *buffer = temp
    add r1, r1, #1      ; buffer++
    bal itoua_return    ; Branch back to return section


; ------------------------------------------------------------------------------
;   void print(void*)
;   r0          : void* buffer, Pointer to buffer
print:
    push lp
    push r7
    mov r3, r0          ; r3 = buffer
    bwl strlen          ; strlen(buffer)
    mov r2, r0
    mov r1, r3
    mov r0, #1          ; fd = stdout
    mov r7, #4          ; r7 = 0x4 (write)
    swi #0              ; syscall write(stdout, &string, len)
    pop r7
    pop lp
    mov pc, lp          ; return


; ------------------------------------------------------------------------------
;   int strlen(void*)
;   Counts the length of a null-terminated string
;   r0          : void* buffer, Pointer to buffer
;   return(r0)  : int length, Length of buffer
strlen:
    mov r1, #-1
    strlen_loop:
    add r1, r1, #1
    ldrb r2, r0, r1
    cmp r2, #0
    bne strlen_loop
    mov r0, r1
    mov pc, lp          ; return
