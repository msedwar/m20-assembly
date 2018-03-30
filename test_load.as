section .text
_part1:
    noop
    ldr r2, _word
    ldr r2, r1
    ldr r2, r0, _word
    ldr r2, r0, r1

section .text
    noop
    ldrh r2, _half
    ldrh r2, r1
    ldrh r2, r0, _half
    ldrh r2, r0, r1

section .text
    noop
    ldrb r2, _byte
    ldrb r2, r1
    ldrb r2, r0, _byte
    ldrb r2, r0, r1

section .data
_word: dw 0xFFFFFFFF
_byte: db 0x11
_half: dh 0x2222

_padding:
    db 0x11

section .text
_part2:
    noop
    noop
    ldr r2, _word
    ldr r2, r1
    ldr r2, r0, _word
    ldr r2, r0, r1

section .text
    noop
    noop
    ldrsh r2, _half
    ldrsh r2, r1
    ldrsh r2, r0, _half
    ldrsh r2, r0, r1

section .text
    noop
    noop
    ldrsb r2, _byte
    ldrsb r2, r1
    ldrsb r2, r0, _byte
    ldrsb r2, r0, r1
