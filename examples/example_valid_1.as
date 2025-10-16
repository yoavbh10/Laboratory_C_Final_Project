; file strings_and_data_only.as
.entry TXT
.extern OUT

; Tiny code to touch data & extern so we produce .ent + .ext too
MAIN:   mov  TXT[r0][r0], r1
        prn  r1
        mov  #0, OUT
        stop

TXT:    .mat [1][1]  65        ; 'A'
ARR:    .data 0, -1, 255, -128, 127
MSG:    .string "Hi!"

