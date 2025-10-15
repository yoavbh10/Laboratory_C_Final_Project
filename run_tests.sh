#!/usr/bin/env bash
# Minimal, reliable test runner for the second pass

set -u

banner() {
  echo
  echo "=== $1 ==="
}

# ---- 1) Clean build ---------------------------------------------------------
banner "CLEAN BUILD"
make clean
set -e
make test_second_pass
set +e

# ---- 2) Recreate test .am files each run -----------------------------------

# (a) test_second_pass.am — tiny file with a label reference to test fixups
cat > test_second_pass.am <<'EOF'
.entry MAIN
.extern EXT_LABEL

MAIN:   mov r1, EXT_LABEL
        add r2, MAIN
        stop
EOF

# (b) test_second_pass_valid_test.am — larger, “valid” program (no directives)
cat > test_second_pass_valid_test.am <<'EOF'
; Valid assembler source file (pure instructions only, no directives)
MAIN:       mov  r3, r5
            add  r5, r6
            sub  r1, r2
            lea  LOOP, r4
            not  r7
            clr  r6
            inc  r1
            dec  r2
            jmp  LOOP
            bne  MAIN
            red  r3
            prn  #5
            jsr  SUBROUTINE
            stop

LOOP:       mov  #5, r1
            add  r2, r3
            stop

SUBROUTINE: inc  r0
            dec  r1
            stop
EOF

# (c) test_second_pass_valid_clean.am — simple valid set (no directives)
cat > test_second_pass_valid_clean.am <<'EOF'
; Simple valid instructions (no directives, no labels except where needed)

MAIN:   mov  r3, r5
        add  #5, r2
        sub  r1, r4
        lea  EXT_LABEL, r6
        clr  r7
        not  r5
        inc  r0
        dec  r2
        jmp  EXT_LABEL
        bne  MAIN
        red  r4
        prn  #10
        jsr  EXT_LABEL
        rts
        stop

EXT_LABEL: mov  r2, r3
           add  r1, r7
           sub  r0, r1
           lea  MAIN, r2
           clr  r4
           prn  r5
           rts
           stop
EOF

# (d) test_second_pass_invalid_test.am — intentionally broken
cat > test_second_pass_invalid_test.am <<'EOF'
; This file is intentionally broken to test error detection
MAIN:   mov     r2              ; Missing destination operand
        add     #5,             ; Missing second operand
        sub     unknown_label, r4
        xyz     r1, r2          ; Unknown opcode
        lea     #5, r3          ; lea source can't be immediate
        mov     r1 r2 r3        ; Too many operands
        clr                      ; Missing operand
        stop    r1               ; stop should have no operands
        rts     #1               ; rts should have no operands
        prn     #hello           ; Invalid immediate (not a number)

STR:    .string hello            ; Missing quotes (directive; should be ignored in 2nd pass)
DATA:   .data   10,,20           ; Bad data (directive; ignored here)
LABEL:  .mat    [2][2] 1 2 3     ; Bad matrix (directive; ignored here)
EOF

# ---- 3) Run tests -----------------------------------------------------------

run_case() {
  local title="$1"
  local file="$2"
  banner "$title"
  if [[ ! -f "$file" ]]; then
    echo "Missing file: $file"
    return 1
  fi
  ./test_second_pass "$file"
}

run_case "TEST 1: Undefined label test (test_second_pass.am)"        "test_second_pass.am"
run_case "TEST 2: Valid program (test_second_pass_valid_test.am)"    "test_second_pass_valid_test.am"
run_case "TEST 3: Valid clean program (test_second_pass_valid_clean.am)" "test_second_pass_valid_clean.am"
run_case "TEST 4: Invalid program (test_second_pass_invalid_test.am)"    "test_second_pass_invalid_test.am"

echo
echo "All tests completed."

