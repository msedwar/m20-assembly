; ==============================================================================
; String Library File
;
;   Author:         Matthew Edwards
;   Dependencies:   
; ==============================================================================

; EXPORTS ======================================================================

global itoua
global memcpy
global memset
global strcpy
global strlen


; DEFINITIONS ==================================================================

section .text

; ------------------------------------------------------------------------------
;   void itoua( uint value , void * buffer )
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
;   void * memcpy( void * dest, void * src, uint num )
;   Copies num bytes from src pointer to the dest pointer.
;   r0          : void * ptr, Pointer to destination buffer
;   r1          : void * ptr, Pointer to source buffer
;   r2          : uint num, Length of buffers
;   return(r0)  : Returns destination buffer pointer
memcpy:
    cmp r2, #0
    beq memcpy_return

    memcpy_loop:
    sub.s r2, r2, #1
    ldrb r3, r1, r2
    strb r3, r0, r2
    bne memcpy_loop

    memcpy_return:
    mov pc, lp


; ------------------------------------------------------------------------------
;   void * memset( void * ptr, int val, uint num )
;   Sets the first num bytes of ptr to val. val is passed as an int but will
;       interpreted as an unsigned byte.
;   r0          : void * ptr, Pointer to buffer
;   r1          : int val, Value to which to set memory
;   r2          : uint num, Number of positions to set
;   return(r0)  : Returns ptr
memset:
    cmp r2, #0
    beq memset_return

    memset_loop:
    sub.s r2, r2, #1
    strb r1, r0, r2
    bne memset_loop

    memset_return:
    mov pc, lp


; ------------------------------------------------------------------------------
;   void * strcpy( void * dest, void * src )
;   Copies the C-string pointed to by src into the buffer pointed to by dest
;   r0          : void * dest, Pointer to destination buffer
;   r1          : void * src, Pointer to the source C-string
;   return(r0)  : Returns dest
strcpy:
    mov r2, #-1

    strcpy_loop:
    add r2, r2, #1
    ldrb r3, r1, r2
    strb r3, r0, r2
    cmp r3, #0
    bne strcpy_loop

    mov pc, lp


; ------------------------------------------------------------------------------
;   int strlen( void * buffer )
;   Counts the length of a null-terminated string
;   r0          : void * buffer, Pointer to buffer
;   return(r0)  : int length, Length of buffer
strlen:
    mov r1, #-1
    
    strlen_loop:
    add r1, r1, #1
    ldrb r2, r0, r1
    cmp r2, #0
    bne strlen_loop

    mov r0, r1
    mov pc, lp
