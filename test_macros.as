; test_macros.as - Test for pre-assembler
; should expand macros and replace .define values

.define SIZE = 5
.define VALUE = -3

; Normal code using macros
START:  mov #SIZE, r1
        mov #VALUE, r2

; Macro definition
mcro INIT
        clr r3
        add #SIZE, r3
        prn r3
mcroend

; Use macro twice
        INIT
        INIT

; Normal instructions after macro expansion
        stop

