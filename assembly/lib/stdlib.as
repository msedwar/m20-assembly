; ==============================================================================
; Standard Utilities Library File
;
;   Author:         Matthew Edwards
;   Dependencies:   
; ==============================================================================

; EXPORTS ======================================================================

global salloc
global sfree


; DEFINITIONS ==================================================================

section .text

; ------------------------------------------------------------------------------
;   void * salloc( uint size )
;   Allocates size bytes on the stack. If this causes a stack overflow, the
;       program behavior is undefined
;   r0          : uint size, Number of bytes to allocate on the stack
;   return(r0)  : Returns a pointer to the allocation on the stack
salloc:
    sub sp, sp, r0
    push r0
    add r0, sp, #4
    mov pc, lp


; ------------------------------------------------------------------------------
;   void sfree()
;   Deallocates the memory block on the top of the program stack
sfree:
    pop r12
    add sp, sp, r12
    mov pc, lp

