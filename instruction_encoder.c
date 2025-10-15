#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>

#include "instruction_encoder.h"
#include "instruction_set.h"
#include "symbol_table.h"
#include "error_list.h"
#include "memory_image.h"
#include "addressing_modes.h"

/* Logical base for code addresses */
#define LOGICAL_BASE 100

/* Bit layout (10-bit word):
   [9..6] opcode
   [5..4] src addressing (2 bits)
   [3..2] dst addressing (2 bits)
   [1..0] ARE (A=00, E=01, R=10) */
enum { ARE_A = 0, ARE_E = 1, ARE_R = 2 };

/* ---------- small helpers (ANSI C) ---------- */
static void add_err(ErrorList *errors, int line, const char *fmt, ...) {
    char buf[ERROR_MSG_LEN];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    add_error(errors, line, buf);
}

static void strip_comment(char *s) {
    for (; *s; ++s) if (*s == ';') { *s = '\0'; break; }
}
static char *lstrip(char *s) { while (*s && isspace((unsigned char)*s)) s++; return s; }
static void rstrip_inplace(char *s) {
    size_t n = strlen(s);
    while (n && (s[n-1] == '\n' || s[n-1] == '\r' || isspace((unsigned char)s[n-1]))) s[--n] = '\0';
}
static void trim_inplace(char *s) { char *ls; rstrip_inplace(s); ls = lstrip(s); if (ls != s) memmove(s, ls, strlen(ls)+1); }

/* Tokenize by whitespace/commas (used only to grab optional label + mnemonic) */
static int tokenize(const char *line, char tokens[][31], int max_tokens) {
    int count = 0;
    const char *p = line;
    while (*p && count < max_tokens) {
        int len = 0;
        while (*p && (isspace((unsigned char)*p) || *p == ',')) p++;
        if (!*p) break;
        while (*p && !isspace((unsigned char)*p) && *p != ',' && len < 30) tokens[count][len++] = *p++;
        tokens[count][len] = '\0';
        count++;
    }
    return count;
}

/* directives we ignore in pass-2 */
static int is_directive(const char *tok) {
    if (!tok || tok[0] != '.') return 0;
    return (strcmp(tok, ".data")==0 || strcmp(tok, ".string")==0 ||
            strcmp(tok, ".extern")==0 || strcmp(tok, ".entry")==0);
}

/* ANSI strdup */
static char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = (char*)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

/* split by commas into at most 2 operands; trim; drop empties; never read past end */
static int split_commas_inplace(char *s, char *parts[], int max_parts) {
    int count = 0;
    char *q = s;
    char *end = s + strlen(s);

    while (q < end && count < max_parts) {
        char *start = q;
        char *tok_end;

        while (q < end && *q != ',') q++;
        tok_end = q;

        if (q < end && *q == ',') { *q = '\0'; q++; }

        while (start < tok_end && isspace((unsigned char)*start)) start++;
        while (tok_end > start && isspace((unsigned char)tok_end[-1])) tok_end--;
        *tok_end = '\0';

        if (*start != '\0') {
            parts[count++] = start;
            if (count == 2) break;
        }
    }
    return count;
}

/* local addressing helpers */
static int enc_is_register(const char *s) { return s && s[0]=='r' && s[1]>='0' && s[1]<='7' && s[2]=='\0'; }
static int enc_is_immediate(const char *s) {
    char *e;
    if (!s || s[0] != '#') return 0;
    (void)strtol(s+1, &e, 10);
    return *lstrip(e) == '\0';
}

/* Matrix parser: LABEL[rX][rY], both indexes must be registers r0..r7 */
static int enc_is_matrix(const char *s, char label_out[MAX_LABEL_LEN], int *row_r, int *col_r) {
    const char *lb, *mid, *rb1, *rb2;
    size_t name_len;
    char tmp[64];

    if (!s) return 0;
    lb = strchr(s, '[');
    if (!lb) return 0;
    name_len = (size_t)(lb - s);
    if (name_len == 0 || name_len >= MAX_LABEL_LEN) return 0;

    /* copy label */
    memcpy(label_out, s, name_len);
    label_out[name_len] = '\0';

    /* first [ ... ] */
    mid = strchr(lb+1, ']'); if (!mid) return 0;
    if (mid <= lb+1) return 0;
    if (mid - (lb+1) >= (int)sizeof(tmp)) return 0;
    memcpy(tmp, lb+1, (size_t)(mid - (lb+1))); tmp[mid-(lb+1)] = '\0';
    trim_inplace(tmp);
    if (!enc_is_register(tmp)) return 0;
    *row_r = tmp[1] - '0';

    /* second [ ... ] immediately after */
    rb1 = strchr(mid+1, '['); if (!rb1) return 0;
    rb2 = strchr(rb1+1, ']'); if (!rb2) return 0;
    if (rb2 <= rb1+1) return 0;
    if (rb2 - (rb1+1) >= (int)sizeof(tmp)) return 0;
    memcpy(tmp, rb1+1, (size_t)(rb2 - (rb1+1))); tmp[rb2-(rb1+1)] = '\0';
    trim_inplace(tmp);
    if (!enc_is_register(tmp)) return 0;
    *col_r = tmp[1] - '0';

    /* no trailing junk */
    if (*lstrip((char*)rb2+1) != '\0') return 0;

    return 1;
}

static int detect_mode_lenient(const char *operand) {
    char dummy[MAX_LABEL_LEN];
    int r, c;
    if (!operand || !*operand) return -1;
    if (enc_is_register(operand))  return ADDR_REGISTER;
    if (enc_is_immediate(operand)) return ADDR_IMMEDIATE;
    if (enc_is_matrix(operand, dummy, &r, &c)) return ADDR_MATRIX;
    return ADDR_DIRECT; /* treat as label; fixups resolve it */
}

/* bit packers (ANSI C) */
static int pack_base_word(int opcode,int src_mode,int dst_mode){
    return ((opcode & 0xF)<<6) | ((src_mode & 0x3)<<4) | ((dst_mode & 0x3)<<2) | ARE_A;
}
static int pack_value_word(int value,int are){ return ((value & 0xFF)<<2) | (are & 0x3); }

/* emit matrix extra words: label word (fixup) + packed regs (ARE=A) */
static void emit_matrix_words(const char *operand, MemoryImage *mem, ErrorList *errors, int line_num) {
    char label[MAX_LABEL_LEN];
    int rr, cc;
    (void)errors; (void)line_num;

    if (!enc_is_matrix(operand, label, &rr, &cc)) {
        /* Parser should have validated already; be defensive */
        add_err(errors, line_num, "invalid matrix operand '%s'", operand);
        return;
    }

    /* word 1: address of label (to be fixed up with E/R) */
    {
        int patch_addr = LOGICAL_BASE + mem->IC;
        add_code_word(mem, 0);
        add_fixup(mem, patch_addr, label, line_num);
    }

    /* word 2: packed regs (row in 6..9, col in 2..5) + ARE=A */
    {
        int w = ((rr & 0x7) << 5) | ((cc & 0x7) << 2) | ARE_A;
        add_code_word(mem, w);
    }
}

/* ---------- main API ---------- */
int encode_instruction(const char *line,
                       Symbol *symbols,
                       MemoryImage *mem,
                       ErrorList *errors,
                       int line_num)
{
    char temp[MAX_LINE_LENGTH+4];
    char tokens[6][31];
    int token_count;
    const Instruction *instr;
    const char *op0s, *op1s;
    int src_mode, dst_mode;
    int operand_count;
    int start_index;

    (void)symbols;

    /* prep line */
    strncpy(temp, line, sizeof(temp)-1);
    temp[sizeof(temp)-1] = '\0';
    strip_comment(temp);
    trim_inplace(temp);

    if (temp[0] == '\0') return 1;   /* blank */
    if (temp[0] == '.')  return 1;   /* directive line */

    /* label-only line? */
    {
        size_t len = strlen(temp);
        if (len > 0 && temp[len-1] == ':' &&
            strchr(temp, ' ') == NULL && strchr(temp, '\t') == NULL)
            return 1;
    }

    token_count = tokenize(temp, tokens, 6);
    if (token_count == 0) return 1;

    /* optional leading label */
    start_index = 0;
    if (strchr(tokens[0], ':') != NULL) {
        if (token_count < 2) {
            add_err(errors, line_num, "Label without instruction");
            return 0;
        }
        start_index = 1;
    }

    /* ignore directives after label */
    if (is_directive(tokens[start_index])) return 1;

    /* lookup instruction */
    instr = find_instruction(tokens[start_index]);
    if (!instr) return 1;

    /* ----- parse operands by COMMAS only (safe splitter) ----- */
    {
        char *mn_pos = strstr(temp, tokens[start_index]);
        char *after;
        char *opsbuf;
        char *ops[2];
        int i, nops;

        if (!mn_pos) return 1;
        after = mn_pos + (int)strlen(tokens[start_index]);
        after = lstrip(after);

        opsbuf = xstrdup(after ? after : "");
        if (!opsbuf) { add_err(errors, line_num, "out of memory"); return 0; }

        nops = split_commas_inplace(opsbuf, ops, 2);
        for (i = 0; i < nops; i++) trim_inplace(ops[i]);

        if (instr->operands == 1 && nops > 1) nops = 1;

        operand_count = nops;

        if (instr->operands != operand_count) {
            char msg[96];
            sprintf(msg, "Operand count mismatch for %s (expected %d, got %d)",
                    instr->name, instr->operands, operand_count);
            add_err(errors, line_num, msg);
            free(opsbuf);
            return 0;
        }

        /* 0 operands */
        if (operand_count == 0) {
            int base = pack_base_word(instr->opcode, 0, 0);
            add_code_word(mem, base);
            free(opsbuf);
            return 1;
        }

        /* 1 operand (destination) */
        if (operand_count == 1) {
            int base;
            int dst_r, patch_addr;
            long val;
            int dst_mode;

            op1s = ops[0];
            dst_mode = detect_mode_lenient(op1s);
            if (dst_mode < 0 || dst_mode > 3 || !instr->allowed_dst[dst_mode]) {
                add_err(errors, line_num, "Invalid addressing mode for operand");
                free(opsbuf);
                return 0;
            }

            base = pack_base_word(instr->opcode, 0, dst_mode);
            add_code_word(mem, base);

            if (dst_mode == ADDR_REGISTER) {
                dst_r = op1s[1] - '0';
                add_code_word(mem, ((dst_r & 0x7) << 5) | ARE_A);
            } else if (dst_mode == ADDR_IMMEDIATE) {
                val = strtol(op1s + 1, NULL, 10);
                add_code_word(mem, pack_value_word((int)val, ARE_A));
            } else if (dst_mode == ADDR_DIRECT) {
                patch_addr = LOGICAL_BASE + mem->IC;
                add_code_word(mem, 0);
                add_fixup(mem, patch_addr, op1s, line_num);
            } else { /* ADDR_MATRIX */
                emit_matrix_words(op1s, mem, errors, line_num);
            }

            free(opsbuf);
            return 1;
        }

        /* 2 operands (source, destination) */
        op0s = ops[0];
        op1s = ops[1];

        src_mode = detect_mode_lenient(op0s);
        dst_mode = detect_mode_lenient(op1s);

        if (src_mode < 0 || src_mode > 3 || !instr->allowed_src[src_mode]) {
            add_err(errors, line_num, "Invalid addressing mode for source operand");
            free(opsbuf);
            return 0;
        }
        if (dst_mode < 0 || dst_mode > 3 || !instr->allowed_dst[dst_mode]) {
            add_err(errors, line_num, "Invalid addressing mode for destination operand");
            free(opsbuf);
            return 0;
        }

        add_code_word(mem, pack_base_word(instr->opcode, src_mode, dst_mode));

        /* Special case: both registers share one word */
        if (src_mode == ADDR_REGISTER && dst_mode == ADDR_REGISTER) {
            int sr = op0s[1] - '0';
            int dr = op1s[1] - '0';
            add_code_word(mem, ((sr & 0x7) << 5) | ((dr & 0x7) << 2) | ARE_A);
            free(opsbuf);
            return 1;
        }

        /* source extra(s) */
        if (src_mode == ADDR_REGISTER) {
            int r = op0s[1] - '0';
            add_code_word(mem, ((r & 0x7) << 5) | ARE_A);
        } else if (src_mode == ADDR_IMMEDIATE) {
            long v = strtol(op0s + 1, NULL, 10);
            add_code_word(mem, pack_value_word((int)v, ARE_A));
        } else if (src_mode == ADDR_DIRECT) {
            int patch_addr = LOGICAL_BASE + mem->IC;
            add_code_word(mem, 0);
            add_fixup(mem, patch_addr, op0s, line_num);
        } else { /* ADDR_MATRIX */
            emit_matrix_words(op0s, mem, errors, line_num);
        }

        /* destination extra(s) */
        if (dst_mode == ADDR_REGISTER) {
            int r = op1s[1] - '0';
            add_code_word(mem, ((r & 0x7) << 5) | ARE_A);
        } else if (dst_mode == ADDR_IMMEDIATE) {
            long v = strtol(op1s + 1, NULL, 10);
            add_code_word(mem, pack_value_word((int)v, ARE_A));
        } else if (dst_mode == ADDR_DIRECT) {
            int patch_addr = LOGICAL_BASE + mem->IC;
            add_code_word(mem, 0);
            add_fixup(mem, patch_addr, op1s, line_num);
        } else { /* ADDR_MATRIX */
            emit_matrix_words(op1s, mem, errors, line_num);
        }

        free(opsbuf);
        return 1;
    }
}

