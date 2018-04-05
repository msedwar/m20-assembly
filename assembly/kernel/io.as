; ==============================================================================
; Operating System Bootloader
;
;   Author:         Matthew Edwards
;   Dependencies:   
; ==============================================================================

; EXPORTS ----------------------------------------------------------------------

global puts


; Definitions ------------------------------------------------------------------

section .text

; ------------------------------------------------------------------------------
;   int write( int filedes, const void * buffer, uint size )
;   r0          : int filedes, File descriptor of write (0, 1, 2 refer to std
;                   in, out, error, respectively)
;   r1          : const void * buffer, Pointer to buffer of at least size, which
;                   will be written to file
;   r2          : uint size, Number of bytes to write
;   return(r0)  : Returns the number of bytes that were written, returns a
;                   negative number if an error occurred
__write:
    push lp
    push r5

    ; TODO(msedwar): Implement filedes

    cmp r2, #0
    beq __write_return  ; if (size == 0) return

    mov r5, r1          ; buffer
    mov r0, 0x0a        ; set BIOS interrupt to write character
    mov r3, #0          ; i = 0

    __write_loop:
    ldrb r1, r5, r3     ; temp = *(buffer + i)
    add r3, r3, #1      ; ++i
    swi 0x10            ; BIOS video call
    cmp r3, r2
    blt __write_loop    ; while (i < size)

    __write_return:
    pop r5
    pop lp
    mov pc, lp          ; return


; ------------------------------------------------------------------------------
;   int puts( const void * buffer )
;   Prints a C-string to stdout
;   r0          : const void * buffer, Pointer to C-string
puts:
    push lp             ; Save callee-save registers

    mov r2, #-1         ; Perform custom strlen operation
    print_strlen_loop:  ; (prevents multiple register saves and swaps)
    add r2, r2, #1
    ldrb r3, r0, r2
    cmp r3, #0
    bne print_strlen_loop

    mov r1, r0
    mov r0, #1
    bwl __write         ; write(stdout, buffer, len)

    pop lp
    mov pc, lp          ; return
