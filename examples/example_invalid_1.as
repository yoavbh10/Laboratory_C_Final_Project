; invalid_1.as – too many operands and illegal register
MAIN:   mov r1, r2, r3   ; ERROR: too many operands
        add #5, r8       ; ERROR: r8 does not exist
        stop

