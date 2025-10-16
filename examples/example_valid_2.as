; file matrix_and_branches.as
.entry START
.extern EXT

START:  lea  MAT[r1][r2], r3
        mov  r3, MAT[r4][r5]
        add  #7, r3
        cmp  r3, #7
        bne  L1
        jsr  EXT
        jmp  END
L1:     clr  r3
        not  r3
        prn  r3
END:    stop

MAT:    .mat [2][2]  10,11,12,13

