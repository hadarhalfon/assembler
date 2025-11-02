.entry LOOP
.entry LENGTH
.extern L3
.extern W
MAIN: mov r1,r2
add r2,r3
LOOP: jmp UNDEFINED_SYMBOL
prn #-5
sub r1, r4
inc r5
mov r6,r7
bne L3
END: stop
STR: .string "abcdef"
LENGTH: .data 6,-9,15
K: .data 22
M1: .mat [2][2] 1,2,3,4
