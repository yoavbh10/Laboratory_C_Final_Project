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

        /* advance to next comma or end */
        while (q < end && *q != ',') q++;
        tok_end = q;                 /* token is [start, tok_end) */

        /* if at a comma, null-terminate token and advance past it */
        if (q < end && *q == ',') { *q = '\0'; q++; }

        /* trim left */
        while (start < tok_end && isspace((unsigned char)*start)) start++;
        /* trim right */
        while (tok_end > start && isspace((unsigned char)tok_end[-1])) tok_end--;
        *tok_end = '\0';             /* safe: points within s */

        if (*start != '\0') {
            parts[count++] = start;
            if (count == 2) break;   /* at most 2 operands */
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
static int detect_mode_lenient(const char *operand) {
    if (!operand || !*operand) return -1;
    if (enc_is_register(operand))  return ADDR_REGISTER;
    if (enc_is_immediate(operand)) return ADDR_IMMEDIATE;
    if (strchr(operand, '[') || strchr(operand, ']')) return -1; /* no matrix */
    return ADDR_DIRECT; /* treat as label; fixups resolve it */
}

/* bit packers (ANSI C) */
static int pack_base_word(int opcode,int src_mode,int dst_mode){
    return ((opcode & 0xF)<<6) | ((src_mode & 0x3)<<4) | ((dst_mode & 0x3)<<2) | ARE_A;
}
static int pack_value_word(int value,int are){ return ((value & 0xFF)<<2) | (are & 0x3); }

/* ---- fixup emitter: write placeholder and record address of that word ---- */
static void emit_label_ref(MemoryImage *mem, const char *label, int line)
{
    /* write placeholder value word (ARE bits = absolute for now) */
    add_code_word(mem, pack_value_word(0, ARE_A));
    /* record fixup for the word we just wrote (its absolute code address is mem->IC-1) */
    add_fixup(mem, mem->IC - 1, label, line);
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

        nops = split_commas_inplace(opsbuf, ops, 2);  /* max 2, no over-read */
        for (i = 0; i < nops; i++) trim_inplace(ops[i]);

        /* lenient clamp for single-operand mnemonics */
        if (instr->operands == 1 && nops > 1) nops = 1;

        operand_count = nops;

        /* validate operand count */
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
            add_code_word(mem, base);          /* add_code_word bumps IC */
            free(opsbuf);
            return 1;
        }

        /* 1 operand (destination) */
        if (operand_count == 1) {
            int base;
            int dst_r;
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
            } else { /* direct label */
                emit_label_ref(mem, op1s, line_num);
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

        /* reg-reg packs into one word */
        if (src_mode == ADDR_REGISTER && dst_mode == ADDR_REGISTER) {
            int sr = op0s[1] - '0';
            int dr = op1s[1] - '0';
            add_code_word(mem, ((sr & 0x7) << 5) | ((dr & 0x7) << 2) | ARE_A);
            free(opsbuf);
            return 1;
        }

        /* source extra word */
        if (src_mode == ADDR_REGISTER) {
            int r = op0s[1] - '0';
            add_code_word(mem, ((r & 0x7) << 5) | ARE_A);
        } else if (src_mode == ADDR_IMMEDIATE) {
            long v = strtol(op0s + 1, NULL, 10);
            add_code_word(mem, pack_value_word((int)v, ARE_A));
        } else { /* direct label */
            emit_label_ref(mem, op0s, line_num);
        }

        /* destination extra word */
        if (dst_mode == ADDR_REGISTER) {
            int r = op1s[1] - '0';
            add_code_word(mem, ((r & 0x7) << 5) | ARE_A);
        } else if (dst_mode == ADDR_IMMEDIATE) {
            long v = strtol(op1s + 1, NULL, 10);
            add_code_word(mem, pack_value_word((int)v, ARE_A));
        } else { /* direct label */
            emit_label_ref(mem, op1s, line_num);
        }

        free(opsbuf);
        return 1;
    }
}

