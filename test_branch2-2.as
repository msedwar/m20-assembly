global _startE
extern _start

section .text

_startE:
    noop
    noop
    noop
    b _start

section.text

    noop
    b _startE
