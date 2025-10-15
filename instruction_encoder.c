#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "instruction_encoder.h"
#include "instruction_set.h"
#include "addressing_modes.h"
#include "symbol_table.h"
#include "error_list.h"
#include "memory_image.h"

/* ========== Small helpers (ANSI C89) ========== */

static void trim(char *s)
{
    char *p = s;
    char *end;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    if (*s == '\0') return;
    end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
}

/* Lowercase into dst (safe length) */
static void to_lower_copy(char *dst, const char *src, int dst_size)
{
    int i = 0;
    if (dst_size <= 0) return;
    while (src[i] && i < dst_size - 1) {
        char c = src[i];
        if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
        dst[i] = c;
        i++;
    }
    dst[i] = '\0';
}

/* Parse: optional label, opcode, rest-of-line operands
   Returns 1 if opcode found (or line is ignorable), 0 on hard parse error. */
static int parse_line(const char *line,
                      char *label,      /* out (may be "") */
                      char *opcode,     /* out (lowercased) */
                      char *operands)   /* out (raw, may be "") */
{
    /* work buffer */
    char buf[MAX_LINE_LENGTH];
    char *p;
    char *first;
    char *rest;

    label[0] = '\0';
    opcode[0] = '\0';
    operands[0] = '\0';

    /* copy & trim */
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    trim(buf);

    /* empty or comment */
    if (buf[0] == '\0' || buf[0] == ';') return 1;

    /* assembler directive? skip in second pass */
    if (buf[0] == '.') {
        /* no opcode intentionally */
        return 1;
    }

    /* find first token */
    p = buf;
    first = p;
    while (*p && !isspace((unsigned char)*p)) p++;

    if (*p) {
        *p = '\0';
        p++;
    }
    rest = p;
    while (*rest && isspace((unsigned char)*rest)) rest++;

    /* label? */
    if (first[0] != '\0' && first[strlen(first) - 1] == ':') {
        /* store label without colon */
        size_t L = strlen(first);
        if (L > 1) {
            size_t i;
            for (i = 0; i + 1 < L && i < (size_t)MAX_LINE_LENGTH - 1; i++)
                label[i] = first[i];
            label[i] = '\0';
        } else {
            label[0] = '\0';
        }

        /* next token is opcode */
        while (*rest && isspace((unsigned char)*rest)) rest++;
        if (*rest == '\0' || *rest == ';') {
            /* label with no opcode is allowed to be ignored here */
            return 1;
        }

        /* take next token as opcode */
        first = rest;
        p = first;
        while (*p && !isspace((unsigned char)*p)) p++;
        if (*p) {
            *p = '\0';
            p++;
        }
        to_lower_copy(opcode, first, MAX_LINE_LENGTH);

        /* remaining are operands (raw) */
        while (*p && isspace((unsigned char)*p)) p++;
        strncpy(operands, p, MAX_LINE_LENGTH - 1);
        operands[MAX_LINE_LENGTH - 1] = '\0';
        return 1;
    }

    /* first token is opcode */
    to_lower_copy(opcode, first, MAX_LINE_LENGTH);

    /* rest are operands (raw) */
    strncpy(operands, rest, MAX_LINE_LENGTH - 1);
    operands[MAX_LINE_LENGTH - 1] = '\0';
    trim(operands);

    return 1;
}

/* Split "a, b" -> op1="a", op2="b"
   Returns operand count 0..2. Extra commas are treated as errors by caller via count check */
static int split_operands(const char *oper, char *op1, char *op2)
{
    const char *c;
    char left[MAX_LINE_LENGTH];
    char right[MAX_LINE_LENGTH];
    int i;

    op1[0] = '\0';
    op2[0] = '\0';

    if (oper[0] == '\0') return 0;

    /* look for first comma */
    c = oper;
    while (*c && *c != ',') c++;

    if (*c == ',') {
        /* split */
        size_t nleft = (size_t)(c - oper);
        if (nleft >= (size_t)MAX_LINE_LENGTH) nleft = MAX_LINE_LENGTH - 1;
        for (i = 0; i < (int)nleft; i++) left[i] = oper[i];
        left[nleft] = '\0';
        c++; /* skip comma */
        while (*c && isspace((unsigned char)*c)) c++;
        strncpy(right, c, MAX_LINE_LENGTH - 1);
        right[MAX_LINE_LENGTH - 1] = '\0';
        trim(left);
        trim(right);
        if (left[0] != '\0') strncpy(op1, left, MAX_LINE_LENGTH - 1);
        op1[MAX_LINE_LENGTH - 1] = '\0';
        if (right[0] != '\0') strncpy(op2, right, MAX_LINE_LENGTH - 1);
        op2[MAX_LINE_LENGTH - 1] = '\0';

        if (op1[0] == '\0' && op2[0] == '\0') return 0;
        if (op2[0] == '\0') return 1;
        return 2;
    } else {
        /* single operand */
        strncpy(op1, oper, MAX_LINE_LENGTH - 1);
        op1[MAX_LINE_LENGTH - 1] = '\0';
        trim(op1);
        if (op1[0] == '\0') return 0;
        return 1;
    }
}

/* Map operand string -> AddressingType (matches instruction_set.h order) */
static int detect_addressing_mode_full(const char *operand)
{
    if (operand[0] == '\0') return -1;
    if (is_immediate(operand)) return ADDR_IMMEDIATE;   /* #number */
    if (is_register(operand))  return ADDR_REGISTER;    /* r0..r7   */
    /* no matrix syntax support in this stage; treat like label otherwise */
    if (is_label(operand))     return ADDR_DIRECT;      /* label    */
    return -1;
}

/* ========== Encoding ========== */

int encode_instruction(const char *line,
                       Symbol *symbols,      /* not used here; fixups resolved in second_pass */
                       MemoryImage *mem,
                       ErrorList *errors,
                       int line_num)
{
    char label[MAX_LINE_LENGTH];
    char opcode[MAX_LINE_LENGTH];
    char operands[MAX_LINE_LENGTH];
    const Instruction *inst;
    int nops;
    char op1[MAX_LINE_LENGTH];
    char op2[MAX_LINE_LENGTH];

    int mode1 = -1, mode2 = -1;

    (void)symbols; /* silence C89 unused param warning */

    /* Parse line into parts; returns 1 even for ignorable lines */
    if (!parse_line(line, label, opcode, operands)) {
        add_error(errors, line_num, "Parse error");
        return 0;
    }

    /* Ignore: empty, comment, directive, or label-only line */
    if (opcode[0] == '\0')
        return 1;

    /* Find instruction (opcodes stored lowercase in instruction_set.c) */
    inst = find_instruction(opcode);
    if (!inst) {
        add_error(errors, line_num, "Unknown opcode");
        return 0;
    }

    /* Split operands */
    op1[0] = '\0';
    op2[0] = '\0';
    nops = split_operands(operands, op1, op2);

    /* Enforce operand count */
    if (nops != inst->operands) {
        char msg[128];
        sprintf(msg, "Operand count mismatch for %s (expected %d, got %d)",
                opcode, inst->operands, nops);
        add_error(errors, line_num, msg);
        return 0;
    }

    /* Determine addressing modes and validate against instruction table */
    if (inst->operands == 2) {
        mode1 = detect_addressing_mode_full(op1);
        mode2 = detect_addressing_mode_full(op2);
        if (mode1 < 0 || mode2 < 0) {
            add_error(errors, line_num, "Invalid addressing mode for operand(s)");
            return 0;
        }
        if (!inst->allowed_src[mode1] || !inst->allowed_dst[mode2]) {
            add_error(errors, line_num, "Invalid addressing mode for operand(s)");
            return 0;
        }
    } else if (inst->operands == 1) {
        mode2 = detect_addressing_mode_full(op1); /* treat as destination (unary ops) */
        if (mode2 < 0) {
            add_error(errors, line_num, "Invalid addressing mode for operand");
            return 0;
        }
        if (!inst->allowed_dst[mode2]) {
            add_error(errors, line_num, "Invalid addressing mode for operand");
            return 0;
        }
    } else {
        /* 0-operand op (rts/stop) — nothing to validate here */
    }

    /* Emit opcode word (very simple “word” model for now) */
    add_code_word(mem, inst->opcode);

    /* Emit operand words + record fixups for labels */
    if (inst->operands == 2) {
        /* source operand */
        if (mode1 == ADDR_IMMEDIATE) {
            /* parse integer after '#' */
            const char *q = op1;
            long val = 0;
            int sign = 1;
            if (*q == '#') q++;
            if (*q == '+') { sign = 1; q++; }
            else if (*q == '-') { sign = -1; q++; }
            while (*q && isdigit((unsigned char)*q)) {
                val = val * 10 + (*q - '0');
                q++;
            }
            add_code_word(mem, (int)(sign * val));
        } else if (mode1 == ADDR_REGISTER) {
            add_code_word(mem, (int)(op1[1] - '0')); /* r0..r7 => 0..7 */
        } else { /* ADDR_DIRECT (label) */
            int placeholder_addr = mem->IC;
            add_code_word(mem, 0); /* placeholder */
            add_fixup(mem, placeholder_addr, op1, line_num);
        }

        /* destination operand */
        if (mode2 == ADDR_IMMEDIATE) {
            const char *q2 = op2;
            long v2 = 0;
            int s2 = 1;
            if (*q2 == '#') q2++;
            if (*q2 == '+') { s2 = 1; q2++; }
            else if (*q2 == '-') { s2 = -1; q2++; }
            while (*q2 && isdigit((unsigned char)*q2)) {
                v2 = v2 * 10 + (*q2 - '0');
                q2++;
            }
            add_code_word(mem, (int)(s2 * v2));
        } else if (mode2 == ADDR_REGISTER) {
            add_code_word(mem, (int)(op2[1] - '0'));
        } else { /* label */
            int placeholder_addr2 = mem->IC;
            add_code_word(mem, 0);
            add_fixup(mem, placeholder_addr2, op2, line_num);
        }
    } else if (inst->operands == 1) {
        /* single destination operand in op1/mode2 */
        if (mode2 == ADDR_IMMEDIATE) {
            const char *q3 = op1;
            long v3 = 0;
            int s3 = 1;
            if (*q3 == '#') q3++;
            if (*q3 == '+') { s3 = 1; q3++; }
            else if (*q3 == '-') { s3 = -1; q3++; }
            while (*q3 && isdigit((unsigned char)*q3)) {
                v3 = v3 * 10 + (*q3 - '0');
                q3++;
            }
            add_code_word(mem, (int)(s3 * v3));
        } else if (mode2 == ADDR_REGISTER) {
            add_code_word(mem, (int)(op1[1] - '0'));
        } else { /* label */
            int placeholder_addr3 = mem->IC;
            add_code_word(mem, 0);
            add_fixup(mem, placeholder_addr3, op1, line_num);
        }
    } else {
        /* 0-operand: nothing more to emit */
    }

    return 1;
}

