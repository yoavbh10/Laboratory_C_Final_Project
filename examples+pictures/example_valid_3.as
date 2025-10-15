; file: full_sweep_ok.as
; Covers: all opcodes, imm/direct/matrix/register modes, entries/externs, data/string/mat

.entry START
.entry LEN
.extern EXT1
.extern EXT2

; --- code ---
START:  mov  r1, r2
        mov  #5, r3
        mov  MAT[r1][r2], r4
        cmp  #7, r3
        add  r3, r4
        sub  LABEL, r3
        lea  LABEL, r5
        not  r5
        clr  r6
        inc  r6
        dec  r6
        prn  #-5
        red  r0
        jmp  EXT1
        bne  EXT2
        jsr  EXT1
        rts
END:    stop

; branch target + direct addressing
LABEL:  mov  r4, r7
        cmp  r7, r4
        bne  END

; --- data / strings / matrix ---
STR:    .string "OK!"
LEN:    .data 3, -1, 0, 255, -128
MAT:    .mat [2][2]  10, 11, 12, 13

