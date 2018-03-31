; ==============================================================================
; System Call Utilities Library File
;
;   Author:         Matthew Edwards
;   Dependencies:   
; ==============================================================================

; EXPORTS ======================================================================

global exit
global read
global write


; DEFINITIONS ==================================================================

section .text

; ------------------------------------------------------------------------------
;   void exit( int status )
;   r0          : int status, Application exit status
exit:
    mov r7, #1      ; SWI_exit
    swi #0          ; exit(status)
    halt            ; Execution should not reach here


; ------------------------------------------------------------------------------
;   int read( int filedes, const void * buffer, uint size )
;   r0          : int filedes, File descriptor of read (0, 1, 2 refer to std
;                   in, out, error, respectively)
;   r1          : const void * buffer, Pointer to buffer of at least size, which
;                   will be read from file
;   r2          : uint size, Number of bytes to read
;   return(r0)  : Returns the number of bytes that were read, returns a
;                   negative number if an error occurred in the syscall
read:
    push lp         ; Save callee-save registers
    push r7

    mov r7, #2      ; SWI_read
    swi #0          ; read(filedes, buffer, size)

    pop r7          ; Load callee-save registers
    pop lp
    mov pc, lp      ; return


; ------------------------------------------------------------------------------
;   int write( int filedes, const void * buffer, uint size )
;   r0          : int filedes, File descriptor of write (0, 1, 2 refer to std
;                   in, out, error, respectively)
;   r1          : const void * buffer, Pointer to buffer of at least size, which
;                   will be written to file
;   r2          : uint size, Number of bytes to write
;   return(r0)  : Returns the number of bytes that were written, returns a
;                   negative number if an error occurred in the syscall
write:
    push lp         ; Save callee-save registers
    push r7

    mov r7, #3      ; SWI_write
    swi #0          ; write(filedes, buffer, size)

    pop r7          ; Load callee-save registers
    pop lp
    mov pc, lp      ; return

