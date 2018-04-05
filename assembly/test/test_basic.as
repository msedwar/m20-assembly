section .text

noop

add r0, r1, r2
adc r1, r2, r3
sub r2, r3, r4
sbc r3, r4, r5
mul r1, r1, r2
div r1, r1, r2
udv r1, r1, r2

or  r1, r1, r2
and r1, r1, r2
xor r1, r1, r2
nor r1, r1, r2
bic r1, r1, r2
ror r1, r1, r2
lsl r1, r1, r2
lsr r1, r1, r2
asr r1, r1, r2

add r0, r1, #2
adc r1, r2, #3
sub r2, r3, #4
sbc r3, r4, #5
mul r1, r1, #2
div r1, r1, #2
udv r1, r1, #2

mov r1, r2
mov r2, #3
mov r2, 0xFFFF
mvn r1, r2
mvn r2, #3
mvn r2, 0xFFFF

cmp r1, r2
cmp r1, #3
cmn r1, r2
cmn r1, #3

tst r1, r2
tst r1, #3
teq r1, r2
teq r1, #3

push r1
push #2
push 0xFFFFF
pop r1

srl r2, st
srl r3, sv
srs st, r2
srs sv, r3

halt
