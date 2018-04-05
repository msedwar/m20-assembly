section .text

_start:
    noop
    noop
    noop
    noop
    bal _last

_next:
    noop
    noop
    bal _start

_last:
    noop
    noop
    noop
    bal _next

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

_finish:
    b r2
    bwl r3
