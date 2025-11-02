MAIN: mov M1[r2][r7],LENGTH
add r2,STR
LOOP: jmp END
prn #-5
mcro a_mc
mov M1[r3][r3],r3
bne LOOP
mcroend
sub r1,r4
inc K

a_mc
END: stop
STR: .string "abcdef"
LONG: .string "567892123456789312345678941234567895123456789612345678971234567898"
LENGTH: .data 6,-9,15
K: .data 22
M1: .mat [2][2] 1,2,3,4
