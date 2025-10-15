; file ps.as
.entry loop
.entry length
.extern l3
.extern w

main:    mov  m1[r2][r7], w
         add  r2, str
loop:    jmp  w
         prn  #-5
         sub  r1, r4
         inc  k

         mov  m1[r3][r3], r3
         bne  l3
end:     stop
str:     .string "abcdef"
length:  .data 6, -9, 15
k:       .data 22
m1:      .mat [2][2] 1,2,3,4
