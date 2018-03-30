global _start
extern _startE

section .text

_start:
    noop
    noop
    noop
    noop
    bal _last           ; 16

section .text

_next:
    noop
    noop
    bal _start          ; 28

section .text

_last:
    noop
    noop
    noop
    bal _next           ; 44

section .text

_start2:
    noop
    noop
    noop
    noop
    bwl _last2

_next2:
    noop
    noop
    bwl _start2

_last2:
    noop
    noop
    noop
    bwl _next2

section .text

_finish:
    b r2
    bwl r3
    b _startE

section .data
    dw  #5
