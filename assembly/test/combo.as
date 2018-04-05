; ==============================================================================
; Test file 2
; ==============================================================================

extern itoua
extern print

entry _main

; ==============================================================================
; Text
; ==============================================================================
section .text

_main:
    push lp
    mov r4, sp          ; void *buffer
    sub sp, sp, #10     ; buffer = alloc(10)

    ldr r0, _n
    ldr r1, _r
    bwl combo
    mov r5, r2          ; int temp = combo(n, r)

    mov r0, r2
    mov r1, r4
    bwl itoua

    mov r0, r4
    bwl print

    add sp, sp, #10     ; dealloc buffer
    pop lp
    mov r0, #0
    mov pc, lp          ; return 0

; ------------------------------------------------------------------------------
;   Combo
;   Performs a mathematic combination, note non-standard return register
;   r0 - int n
;   r1 - int r
;   r2 - return int
combo:
    cmp r1, #0              ; if (r == 0 || n == r)
    beq combo_default
    cmp r0, r1
    beq combo_default

    push lp                 ; else begin
    push r1                 ; push r
    sub r0, r0, #1
    push r0                 ; push n-1
    bwl combo               ; result1 = combo(n-1, r)
    pop r0                  ; pop n-1
    pop r1                  ; pop r
    push r2                 ; push result1
    sub r1, r1, #1
    bwl combo               ; result2 = combo(n-1, r-1)
    pop r3                  ; pop result1
    add r2, r2, r3          ; result1 + result2
    pop lp
    mov pc, lp              ; return

    combo_default:          ; if begin
    mov r2, #1
    mov pc, lp              ; return 1


; ==============================================================================
; Data
; ==============================================================================
section .data

_n: dw #31
_r: dw #15
