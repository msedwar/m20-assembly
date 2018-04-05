; ==============================================================================
; Test file 3
; ==============================================================================

extern itoua
extern print

entry main

; TEXT =========================================================================
section .text

; ------------------------------------------------------------------------------
;   int main()
;   Main entrypoint function
main:
    push lp
    sub sp, sp, #10     ; buf = alloc(10)
    mov r4, sp          ; void *buf
    sub sp, sp, #4      ; increment stack
    ldr r5, _num        ; int num

    mov r0, _string
    bwl print           ; print(&_string)

    mov r0, r5
    mov r1, r4
    bwl itoua           ; itoua(20, buf)

    mov r0, r4
    bwl print           ; print(buf)

    mov r0, _string2
    bwl print           ; print(&_string2)

    mov r0, r5
    bwl fib             ; temp = fib(20) [=6765]

    mov r1, r4
    bwl itoua           ; itoua(temp, buf)

    mov r0, r4
    bwl print           ; print(buf)

    add sp, sp, #14     ; deallocate buf

    pop lp
    mov r0, #0
    mov pc, lp          ; return 0


; ------------------------------------------------------------------------------
;   int fib(int)
;   Find the n-th Fibonnaci number
;   r0          : uint pos, Position of the Fibonnaci number to generate
;   return(r0)  : uint num, Fibonnaci number
fib:
    cmp r0, #0
    moveq r0, #0
    moveq pc, lp        ; if (pos == 0) return 0
    cmp r0, #1
    moveq pc, lp        ; if (pos == 1) return 1
    push lp
    sub r0, r0, #1
    push r0
    bwl fib             ; temp1 = fib(pos - 1)
    pop r1
    push r0
    mov r0, r1
    sub r0, r0, #1
    bwl fib             ; temp2 = fib(pos - 2)
    pop r1
    add r0, r0, r1
    pop lp
    mov pc, lp          ; return temp1 + temp2


; DATA =========================================================================
section .data

_string:
    db "Hello World!\n\nFibonnaci (\0"
    space #2
_string2:
    db "): \0"
_num:
    dw #25
