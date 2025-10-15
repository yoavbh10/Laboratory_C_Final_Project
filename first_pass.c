/* first_pass.c — ANSI C (C90)
   Pass 1: build symbol table, data image, and compute code size (IC).
   Now supports .data, .string, .mat directives.
   Addressing supported in sizing/validation: #immediate, LABEL (direct),
   register r0..r7, and (for sizing/validation only) matrix operands are
   validated in the encoder during pass-2; here we only ensure operand
   counts and label binding so IC stays consistent.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "first_pass.h"
#include "instruction_set.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

#define LOGICAL_BASE 100

/* ---------- small helpers ---------- */
static void add_err(ErrorList *errors, int line, const char *fmt, ...) {
    char buf[ERROR_MSG_LEN];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    add_error(errors, line, buf);
}
static char *lstrip(char *s) { while (*s && isspace((unsigned char)*s)) s++; return s; }
static void rstrip_inplace(char *s) {
    size_t n = strlen(s);
    while (n && (s[n-1]=='\n' || s[n-1]=='\r' || isspace((unsigned char)s[n-1]))) s[--n]='\0';
}
static void trim_inplace(char *s) { char *ls; rstrip_inplace(s); ls=lstrip(s); if (ls!=s) memmove(s, ls, strlen(ls)+1); }
static void strip_comment(char *s) { for(;*s;++s){ if(*s==';'){*s='\0'; break;} } }
static int  is_blank(const char *s){ while(*s){ if(!isspace((unsigned char)*s)) return 0; s++; } return 1; }
static char *xstrdup(const char *s){ size_t n=strlen(s)+1; char *p=(char*)malloc(n); if(p) memcpy(p,s,n); return p; }

/* split by commas into at most N parts; trim; drop empties; never read past end */
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
            if (count == max_parts) break;
        }
    }
    return count;
}

/* read first non-space token from line into out; return 1 if any */
static int peek_first_token(const char *line, char *out, size_t out_sz) {
    size_t i = 0;
    while (*line && isspace((unsigned char)*line)) line++;
    if (!*line) return 0;
    while (*line && !isspace((unsigned char)*line) && *line != ',') {
        if (i+1 < out_sz) out[i++] = *line;
        line++;
    }
    out[i] = '\0';
    return (int)(i > 0);
}

/* .string "foo" → bytes including terminating zero (caller frees *out) */
static int parse_string_literal(const char *s, unsigned char **out, size_t *out_len) {
    const char *start;
    size_t n, i;
    unsigned char *buf;

    s = lstrip((char*)s);
    if (*s != '"') return 0;
    s++;
    start = s;
    while (*s && *s!='"') s++;
    if (*s != '"') return 0;
    n = (size_t)(s - start);
    buf = (unsigned char*)malloc(n + 1);
    if (!buf) return 0;
    for (i=0;i<n;i++) buf[i] = (unsigned char)start[i];
    buf[n] = 0;
    *out = buf; *out_len = n + 1;
    return 1;
}

/* ---------- label/name helpers ---------- */
static int is_register_name(const char *s){ return s && s[0]=='r' && s[1]>='0' && s[1]<='7' && s[2]=='\0'; }
static int is_directive_tok(const char *tok){
    return strcmp(tok,".data")==0 || strcmp(tok,".string")==0 ||
           strcmp(tok,".extern")==0 || strcmp(tok,".entry")==0 ||
           strcmp(tok,".mat")==0; /* added .mat */
}
static int is_valid_label_name(const char *name){
    size_t i, n = strlen(name);
    if (n==0 || n>=MAX_LABEL_LEN) return 0;
    if (!isalpha((unsigned char)name[0])) return 0;
    for (i=1;i<n;i++) if (!isalnum((unsigned char)name[i]) && name[i] != '_') return 0; /* allow '_' */
    if (is_register_name(name)) return 0;
    if (is_directive_tok(name)) return 0;
    if (find_instruction(name) != NULL) return 0;
    return 1;
}

/* read leading "<label>:" if present; advance *p; copy name */
static int take_leading_label(char **p, char out_label[MAX_LABEL_LEN]){
    char *s = *p;
    size_t i = 0;
    s = lstrip(s);
    while (s[i] && !isspace((unsigned char)s[i]) && s[i] != ':') i++;
    if (s[i] != ':') return 0;
    if (i == 0 || i >= MAX_LABEL_LEN) return 0;
    memcpy(out_label, s, i); out_label[i] = '\0';
    if (!is_valid_label_name(out_label)) return 0;
    *p = s + i + 1;
    return 1;
}

/* ---------- directives: .data / .string ---------- */
static void handle_data(MemoryImage *mem, ErrorList *errors, Symbol **symtab, int line,
                        const char *label_opt, char *args){
    char *parts[512];
    int n, i;

    if (label_opt && *label_opt) {
        Symbol *ex = find_symbol(*symtab, label_opt);
        if (ex) {
            if (ex->is_extern) add_err(errors,line,"label '%s' cannot redefine extern",label_opt);
            else if (ex->address != 0) add_err(errors,line,"duplicate label '%s'",label_opt);
            else { ex->address = mem->DC; ex->type = SYMBOL_DATA; }
        } else add_symbol(symtab, label_opt, mem->DC, SYMBOL_DATA);
    }

    if (!args || is_blank(args)) { add_err(errors,line,".data: missing numbers"); return; }

    n = split_commas_inplace(args, parts, 512);
    if (n == 0) { add_err(errors, line, ".data: no numbers"); return; }

    for (i=0;i<n;i++) {
        char *tok = lstrip(parts[i]);
        char *endp = NULL;
        long v;
        if (*tok=='\0') { add_err(errors,line,".data: empty item"); continue; }
        v = strtol(tok, &endp, 10);
        if (endp == tok || *lstrip(endp) != '\0') { add_err(errors,line,".data: invalid integer '%s'", tok); continue; }
        add_data_word(mem, (int)v); /* add_data_word bumps DC */
    }
}

static void handle_string(MemoryImage *mem, ErrorList *errors, Symbol **symtab, int line,
                          const char *label_opt, char *args){
    unsigned char *bytes = NULL;
    size_t n = 0, i;

    if (label_opt && *label_opt) {
        Symbol *ex = find_symbol(*symtab, label_opt);
        if (ex) {
            if (ex->is_extern) add_err(errors,line,"label '%s' cannot redefine extern",label_opt);
            else if (ex->address != 0) add_err(errors,line,"duplicate label '%s'",label_opt);
            else { ex->address = mem->DC; ex->type = SYMBOL_DATA; }
        } else add_symbol(symtab, label_opt, mem->DC, SYMBOL_DATA);
    }

    if (!parse_string_literal(args, &bytes, &n)) { add_err(errors,line,".string: expected quoted string"); return; }
    for (i=0;i<n;i++) add_data_word(mem, (int)bytes[i]); /* DC bumped inside */
    free(bytes);
}

/* ---------- .extern / .entry ---------- */
static void handle_extern(Symbol **symtab, ErrorList *errors, int line, char *args){
    char name[MAX_LABEL_LEN]={0};
    Symbol *existing;

    if (!peek_first_token(args, name, sizeof(name))) { add_err(errors,line,".extern: missing symbol name"); return; }
    if (!is_valid_label_name(name)) { add_err(errors,line,".extern: invalid name '%s'",name); return; }

    existing = find_symbol(*symtab, name);
    if (existing) {
        if (existing->address != 0) { add_err(errors,line,".extern: symbol '%s' already defined",name); return; }
        existing->is_extern = 1;
    } else {
        add_symbol(symtab, name, 0, SYMBOL_CODE);
        existing = find_symbol(*symtab, name);
        if (existing) existing->is_extern = 1;
    }
}

static void handle_entry(Symbol **symtab, ErrorList *errors, int line, char *args){
    char name[MAX_LABEL_LEN]={0};
    Symbol *sym;

    if (!peek_first_token(args, name, sizeof(name))) { add_err(errors,line,".entry: missing symbol name"); return; }
    if (!is_valid_label_name(name)) { add_err(errors,line,".entry: invalid name '%s'",name); return; }
    sym = find_symbol(*symtab, name);
    if (!sym) { add_symbol(symtab, name, 0, SYMBOL_CODE); sym = find_symbol(*symtab, name); }
    if (sym) sym->is_entry = 1;
}

/* ---------- .mat support ---------- */
/* parse [rows][cols], returns 1 on success; *after points past the second ] */
static int parse_mat_dims(char *p, int *rows, int *cols, char **after) {
    char *q = p;
    long r, c;
    char *e;

    q = lstrip(q);
    if (*q != '[') return 0;
    q++;
    r = strtol(q, &e, 10);
    if (e == q || *e != ']') return 0;
    q = e + 1;

    q = lstrip(q);
    if (*q != '[') return 0;
    q++;
    c = strtol(q, &e, 10);
    if (e == q || *e != ']') return 0;
    q = e + 1;

    if (r <= 0 || c <= 0) return 0;
    *rows = (int)r; *cols = (int)c;
    if (after) *after = q;
    return 1;
}

static void handle_mat(MemoryImage *mem, ErrorList *errors, Symbol **symtab, int line,
                       const char *label_opt, char *args)
{
    int rows=0, cols=0, total, i, n=0;
    char *p = args;
    char *list = NULL;
    char *parts[512];

    if (label_opt && *label_opt) {
        Symbol *ex = find_symbol(*symtab, label_opt);
        if (ex) {
            if (ex->is_extern) add_err(errors,line,"label '%s' cannot redefine extern",label_opt);
            else if (ex->address != 0) add_err(errors,line,"duplicate label '%s'",label_opt);
            else { ex->address = mem->DC; ex->type = SYMBOL_DATA; }
        } else add_symbol(symtab, label_opt, mem->DC, SYMBOL_DATA);
    }

    if (!parse_mat_dims(p, &rows, &cols, &p)) { add_err(errors,line,".mat: expected [rows][cols]"); return; }
    total = rows * cols;

    /* optional initializer list after dims */
    p = lstrip(p);
    if (*p) {
        list = xstrdup(p);
        if (!list) { add_err(errors,line,"out of memory"); return; }
        n = split_commas_inplace(list, parts, 512);
    }

    for (i=0;i<total;i++) {
        if (i < n) {
            char *tok = lstrip(parts[i]);
            char *endp = NULL;
            long v = strtol(tok, &endp, 10);
            if (endp == tok || *lstrip(endp) != '\0') {
                add_err(errors,line,".mat: invalid integer '%s'", tok);
                v = 0;
            }
            add_data_word(mem, (int)v);
        } else {
            add_data_word(mem, 0);
        }
    }

    if (list) free(list);
}

/* ---------- addressing & sizing for instructions ---------- */
typedef enum {
    AM_INVALID  = -1,
    AM_IMM      = ADDR_IMMEDIATE, /* 0 */
    AM_DIR      = ADDR_DIRECT,    /* 1 */
    AM_REG      = ADDR_REGISTER   /* 3 (no explicit matrix mode parsed here) */
} AddrMode;

static AddrMode parse_operand(const char *op, char label_out[MAX_LABEL_LEN], long *imm_out){
    char *endp;
    if (!op) return AM_INVALID;
    while (*op && isspace((unsigned char)*op)) op++;

    if (*op == '#') {
        long v = strtol(op+1, &endp, 10);
        if (endp && *lstrip(endp) == '\0') { if (imm_out) *imm_out = v; return AM_IMM; }
        return AM_INVALID;
    }
    if (is_register_name(op)) return AM_REG;

    /* Matrix text is validated/encoded in pass-2; here treat 'LABEL[...][...]' as a label
       for sizing purposes in pass-1 (adds one extra word like direct). */
    if (strchr(op,'[') && strchr(op,']')) {
        /* Take prefix up to '[' as a candidate label; don't need it here. */
        return AM_DIR;
    }

    if (is_valid_label_name(op)) {
        if (label_out) { strncpy(label_out, op, MAX_LABEL_LEN-1); label_out[MAX_LABEL_LEN-1]='\0'; }
        return AM_DIR;
    }
    return AM_INVALID;
}

static int mode_allowed(const Instruction *idef, int op_index, AddrMode m){
    int idx = (int)m;
    if (idx < 0 || idx > 3) return 0;
    if (idef->operands == 2) return (op_index==0) ? (idef->allowed_src[idx]!=0) : (idef->allowed_dst[idx]!=0);
    if (idef->operands == 1) return (idef->allowed_dst[idx]!=0);
    return 0;
}

/* base=1 + extras per operand; if two registers (2-op) → pack into one extra word */
static int compute_words(const Instruction *idef, AddrMode modes[], size_t nops){
    int words = 1;
    size_t i;
    (void)idef;
    if (nops == 0) return words;
    if (nops == 2 && modes[0]==AM_REG && modes[1]==AM_REG) return words + 1;
    for (i=0;i<nops;i++){
        switch (modes[i]) { case AM_IMM: case AM_DIR: case AM_REG: words += 1; break; default: break; }
    }
    return words;
}

/* ---------- instruction handling (sizing + label binding) ---------- */
static void handle_instruction(MemoryImage *mem, ErrorList *errors, Symbol **symtab, int line,
                               const char *label_opt, char *cursor)
{
    char mnemonic[16] = {0};
    const Instruction *idef;
    char *s, *opsbuf;
    char *ops[2] = {0, 0};
    int nops = 0, i;
    AddrMode modes[2] = { AM_INVALID, AM_INVALID };
    char labels[2][MAX_LABEL_LEN];
    long imms[2];
    int words;

    memset(labels, 0, sizeof(labels));
    memset(imms, 0, sizeof(imms));

    if (!peek_first_token(cursor, mnemonic, sizeof(mnemonic))) {
        add_err(errors, line, "expected instruction mnemonic");
        return;
    }
    idef = find_instruction(mnemonic);
    if (!idef) {
        add_err(errors, line, "unknown instruction '%s'", mnemonic);
        return;
    }

    /* take text after mnemonic */
    s = cursor;
    s = lstrip(s);
    s += (int)strlen(mnemonic);
    s = lstrip(s);

    opsbuf = xstrdup(s ? s : "");
    if (!opsbuf) {
        add_err(errors, line, "out of memory");
        return;
    }

    if (!is_blank(opsbuf)) {
        nops = split_commas_inplace(opsbuf, ops, 2);   /* split safely, max 2 */
        for (i = 0; i < nops; i++) trim_inplace(ops[i]);
    }

    /* lenient clamp to avoid false positives from stray commas */
    if (idef->operands == 1 && nops > 1) nops = 1;

    if (nops != idef->operands) {
        add_err(errors, line, "operand count mismatch for '%s' (expected %d, got %d)",
                mnemonic, idef->operands, nops);
        /* continue to compute size so IC stays consistent */
    }

    for (i = 0; i < nops; i++) {
        modes[i] = parse_operand(ops[i], labels[i], &imms[i]);
        if (modes[i] == AM_INVALID) {
            add_err(errors, line, "invalid operand '%s'", ops[i]);
        } else if (!mode_allowed(idef, i, modes[i])) {
            add_err(errors, line, "illegal addressing mode for operand %d on '%s'", i, mnemonic);
        }
    }

    /* bind label to logical code address before sizing */
    if (label_opt && *label_opt) {
        Symbol *ex = find_symbol(*symtab, label_opt);
        if (ex) {
            if (ex->is_extern) {
                add_err(errors, line, "label '%s' cannot redefine extern", label_opt);
            } else if (ex->address != 0) {
                add_err(errors, line, "duplicate label '%s'", label_opt);
            } else {
                ex->address = LOGICAL_BASE + mem->IC;
                ex->type = SYMBOL_CODE;
            }
        } else {
            add_symbol(symtab, label_opt, LOGICAL_BASE + mem->IC, SYMBOL_CODE);
        }
    }

    /* size only (words in code); add_code_word is NOT used in pass 1 */
    words = compute_words(idef, modes, (size_t)nops);
    mem->IC += words;

    free(opsbuf);
}

/* ---------- end-of-pass helpers ---------- */
static void bump_data_symbols_by_icf(Symbol *head, int icf_words){
    int bump = LOGICAL_BASE + icf_words;
    Symbol *s = head;
    for (; s; s = s->next) if (s->type==SYMBOL_DATA && !s->is_extern) s->address += bump;
}

/* ---------- entry point ---------- */
int first_pass(const char *filename, Symbol **symtab, MemoryImage *mem, ErrorList *errors){
    FILE *fp = fopen(filename, "r");
    char linebuf[1024];
    int line_no = 0;

    if (!fp) { add_err(errors,0,"cannot open '%s'",filename); return 0; }

    while (fgets(linebuf, sizeof(linebuf), fp)) {
        size_t raw_len;
        char *cursor;
        char label[MAX_LABEL_LEN]={0};
        int has_label;
        char tok[32]={0};

        line_no++;

        raw_len = strcspn(linebuf, "\r\n");
        if (raw_len > MAX_LINE_LENGTH) add_err(errors,line_no,"line too long (> %d chars)", MAX_LINE_LENGTH);

        strip_comment(linebuf);
        trim_inplace(linebuf);
        if (*linebuf == '\0') continue;

        cursor = linebuf;
        has_label = take_leading_label(&cursor, label);

        if (!peek_first_token(cursor, tok, sizeof(tok))) {
            if (has_label) add_err(errors,line_no,"label with no statement");
            continue;
        }

        if (is_directive_tok(tok)) {
            cursor = lstrip(cursor);
            cursor += (int)strlen(tok);

            if (strcmp(tok,".data")==0)
                handle_data(mem,errors,symtab,line_no,has_label?label:NULL,cursor);
            else if (strcmp(tok,".string")==0)
                handle_string(mem,errors,symtab,line_no,has_label?label:NULL,cursor);
            else if (strcmp(tok,".mat")==0)
                handle_mat(mem,errors,symtab,line_no,has_label?label:NULL,cursor);
            else if (strcmp(tok,".extern")==0) {
                if (has_label) add_err(errors,line_no,"label before .extern is ignored");
                handle_extern(symtab,errors,line_no,cursor);
            } else if (strcmp(tok,".entry")==0) {
                if (has_label) add_err(errors,line_no,"label before .entry is ignored");
                handle_entry(symtab,errors,line_no,cursor);
            }
        } else {
            handle_instruction(mem,errors,symtab,line_no,has_label?label:NULL,cursor);
        }
    }

    fclose(fp);
    bump_data_symbols_by_icf(*symtab, mem->IC);
    return 1;
}

