; ==============================================================================
; String Library File
;
;   Author:         Matthew Edwards
;   Dependencies:   
; ==============================================================================

; EXPORTS ======================================================================

global itoa
global memcpy
global memset
global strcat
global strcpy
global strncpy
global strlen


; DEFINITIONS ==================================================================

section .text


; ------------------------------------------------------------------------------
;   void * itoa( int value, void * buffer, int base )
;   Converts an integer value into a null-terminated string using the specified
;       base and stores the result in buffer. If the base is 10 and value is
;       negative, the number will be preceded by a minus sign. Otherwise, value
;       is considered unsigned. buffer should be long enough to contain any
;       possible value generated
;   r0          : int value, Integer value to convert to string
;   r1          : void * buffer, Pointer to buffer
;   r2          : int base, Integral radix to use in conversion
itoa:
    cmp r0, #0          ; if (r0 == 0) // special case
    beq itoa_put_zero

    push r4             ; Save callee-save registers
    mov r3, #0          ; i = 0
    mov r4, #0          ; isNegative = false

    bgt itoa_loop       ; Check if negative (base-10 numbers only)
    cmp r2, #10
    bne itoa_loop
    mov r4, #1          ; isNegative = true
    mul r0, r0, #-1     ; value = -value

    itoa_loop:
    udv r12, r0, r2
    mul r12, r12, r2
    sub r12, r0, r12    ; r = value % base
    udv r0, r0, r2      ; value /= base

    cmp r12, #9         ; if r > 9
    ble itoa_put_byzero
    add r12, r12, 0x57  ; temp = 'a' + temp
    bal itoa_cond

    itoa_put_byzero:
    add r12, r12, 0x30  ; temp = '0' + temp

    itoa_cond:
    strb r12, r1, r3    ; buffer[i] = temp
    add r3, r3, #1      ; ++i
    cmp r0, #0
    bgt itoa_loop

    cmp r4, #0
    beq itoa_put_term
    mov r12, 0x2D       ; temp = '-'
    strb r12, r1, r3    ; buffer[i] = temp
    add r3, r3, #1      ; ++i

    itoa_put_term:
    mov r12, #0
    strb r12, r1, r3    ; buffer[i] = 0

    add r2, r1, r3
    sub r2, r2, #1      ; r2 = buffer + i

    itoa_reverse:       ; Reverse string
    ldrb r3, r2
    ldrb r12, r1
    strb r12, r2
    strb r3, r1
    add r1, r1, #1
    sub r2, r2, #1
    cmp r2, r1
    bgt itoa_reverse

    pop r4              ; Restore callee-save registers
    mov pc, lp          ; return

    itoa_put_zero:
    mov r12, 0x3000     ; temp = "0\0"
    strh r12, r1        ; *buffer = temp
    mov pc, lp


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
    ldrb r12, r1, r2
    strb r12, r0, r2
    bgt memcpy_loop

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
    bgt memset_loop

    memset_return:
    mov pc, lp


; ------------------------------------------------------------------------------
;   void * strcat( void * dest, const void * src )
;   Concatenates the C-string pointed to by src to the C-string pointed to by
;       the dest buffer. The null-terminator in dest is overwritten and a new
;       one is appended after the concatenation of src.
;   r0          : void * dest, Pointer to destination buffer
;   r1          : const void * src, Pointer to the source C-string
;   return(r0)  : Returns dest
strcat:
    mov r2, #-1             ; Custom strlen function (prevents register swaps)
    strcat_len_loop:        ; r2 = strlen(r0)
    add r2, r2, #1
    ldrb r12, r0, r2
    cmp r12, #0
    bgt strcat_len_loop

    mov r3, #0              ; Custom strcpy function (prevents register swaps)
    strcat_cpy_loop:        ; *(r0 + r2) = strcpy(r1)
    ldrb r12, r1, r3
    strb r12, r0, r2
    add r3, r3, #1
    add r2, r2, #1
    cmp r12, #0
    bgt strcat_cpy_loop

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
    ldrb r12, r1, r2
    strb r12, r0, r2
    cmp r12, #0
    bgt strcpy_loop

    mov pc, lp


; ------------------------------------------------------------------------------
;   void * strncpy( void * dest, void * src, uint num )
;   Copies the C-string pointed to by src into the buffer pointed to by dest
;   r0          : void * dest, Pointer to destination buffer
;   r1          : void * src, Pointer to the source C-string
;   r2          : uint num, Maximum number of characters to be copied from src
;   return(r0)  : Returns dest
strncpy:
    mov r3, #0              ; index = 0

    cmp r2, r3
    ble strncpy_return

    strncpy_loop:
    ldrb r12, r1, r3
    strb r12, r0, r3
    cmp r12, #0             ; if (src[index] == 0) return
    beq strncpy_return

    add r3, r3, #1          ; ++index
    cmp r2, r3              ; if (index >= num) return
    bgt strncpy_loop

    strncpy_return:
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
    ldrb r12, r0, r1
    cmp r12, #0
    bgt strlen_loop

    mov r0, r1
    mov pc, lp
