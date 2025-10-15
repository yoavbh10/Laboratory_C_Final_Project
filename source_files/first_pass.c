/* first_pass.c
 * Pass 1: parse source, collect symbols/data, and compute code size (IC).
 * Validates labels/directives and infers addressing for sizing only.
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
#include "addressing_modes.h"

#define LOGICAL_BASE 100

/* ---------- small helpers ---------- */

/* add_err — printf-style add_error */
static void add_err(ErrorList *errors, int line, const char *fmt, ...) {
    char buf[ERROR_MSG_LEN];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    add_error(errors, line, buf);
}

/* lstrip — skip leading spaces */
static char *lstrip(char *s) { while (*s && isspace((unsigned char)*s)) s++; return s; }

/* rstrip_inplace — trim trailing spaces/newlines */
static void rstrip_inplace(char *s) {
    size_t n = strlen(s);
    while (n && (s[n-1]=='\n' || s[n-1]=='\r' || isspace((unsigned char)s[n-1]))) s[--n]='\0';
}

/* trim_inplace — strip both ends */
static void trim_inplace(char *s) { char *ls; rstrip_inplace(s); ls=lstrip(s); if (ls!=s) memmove(s, ls, strlen(ls)+1); }

/* strip_comment — cut ';' to end */
static void strip_comment(char *s) { for(;*s;++s){ if(*s==';'){*s='\0'; break;} } }

/* is_blank — all whitespace? */
static int  is_blank(const char *s){ while(*s){ if(!isspace((unsigned char)*s)) return 0; s++; } return 1; }

/* xstrdup — ANSI-safe strdup */
static char *xstrdup(const char *s){ size_t n=strlen(s)+1; char *p=(char*)malloc(n); if(p) memcpy(p,s,n); return p; }

/* split_commas_inplace — split into <=2 trimmed parts */
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

/* parse_string_literal — ".string" -> bytes+NUL (malloc'd) */
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

/* is_register_name — r0..r7 */
static int is_register_name(const char *s){ return s && s[0]=='r' && s[1]>='0' && s[1]<='7' && s[2]=='\0'; }

/* is_directive_tok — known directives */
static int is_directive_tok(const char *tok){
    return strcmp(tok,".data")==0 || strcmp(tok,".string")==0 || strcmp(tok,".extern")==0 ||
           strcmp(tok,".entry")==0 || strcmp(tok,".mat")==0;
}

/* is_valid_label_name — letters/digits, not reserved */
static int is_valid_label_name(const char *name){
    size_t i, n = strlen(name);
    if (n==0 || n>=MAX_LABEL_LEN) return 0;
    if (!isalpha((unsigned char)name[0])) return 0;
    for (i=1;i<n;i++) if (!isalnum((unsigned char)name[i])) return 0; /* ONLY letters/digits */
    if (is_register_name(name)) return 0;
    if (is_directive_tok(name)) return 0;
    if (find_instruction(name) != NULL) return 0;
    return 1;
}

/* take_leading_label — parse "<label>:" at start if present */
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

/* ---------- addressing parse (for sizing only) ---------- */

typedef enum {
    AM_INVALID  = -1,
    AM_IMM      = ADDR_IMMEDIATE, /* 0 */
    AM_DIR      = ADDR_DIRECT,    /* 1 */
    AM_MAT      = ADDR_MATRIX,    /* 2 */
    AM_REG      = ADDR_REGISTER   /* 3 */
} AddrMode;

/* enc_is_matrix — quick validator for LABEL[rX][rY] form */
static int enc_is_matrix(const char *s) {
    const char *lb, *mid, *rb1, *rb2;
    char tmp[64];
    if (!s) return 0;
    lb = strchr(s, '['); if (!lb) return 0;
    mid = strchr(lb+1, ']'); if (!mid) return 0;
    if (mid <= lb+1) return 0;
    if (mid - (lb+1) >= (int)sizeof(tmp)) return 0;
    memcpy(tmp, lb+1, (size_t)(mid - (lb+1))); tmp[mid-(lb+1)] = '\0';
    if (!is_register_name(tmp)) return 0;
    rb1 = strchr(mid+1, '['); if (!rb1) return 0;
    rb2 = strchr(rb1+1, ']'); if (!rb2) return 0;
    if (rb2 <= rb1+1) return 0;
    if (rb2 - (rb1+1) >= (int)sizeof(tmp)) return 0;
    memcpy(tmp, rb1+1, (size_t)(rb2 - (rb1+1))); tmp[rb2-(rb1+1)] = '\0';
    if (!is_register_name(tmp)) return 0;
    if (*lstrip((char*)rb2+1) != '\0') return 0;
    return 1;
}

/* parse_operand_mode — detect addressing (lenient) */
static AddrMode parse_operand_mode(const char *op){
    char *endp;
    if (!op) return AM_INVALID;
    while (*op && isspace((unsigned char)*op)) op++;

    if (*op == '#') {
        (void)strtol(op+1, &endp, 10);
        return (*op && endp && *lstrip(endp) == '\0') ? AM_IMM : AM_INVALID;
    }
    if (is_register_name(op)) return AM_REG;
    if (enc_is_matrix(op)) return AM_MAT;

    /* treat as label if legal label name (no matrix brackets inside) */
    if (strchr(op,'[') || strchr(op,']')) return AM_INVALID;
    return is_valid_label_name(op) ? AM_DIR : AM_INVALID;
}

/* mode_allowed — does idef allow this operand mode? */
static int mode_allowed(const Instruction *idef, int op_index, AddrMode m){
    int idx = (int)m;
    if (idx < 0 || idx > 3) return 0;
    if (idef->operands == 2) return (op_index==0) ? (idef->allowed_src[idx]!=0) : (idef->allowed_dst[idx]!=0);
    if (idef->operands == 1) return (idef->allowed_dst[idx]!=0);
    return 0;
}

/* compute_words — base=1 + extras; reg-reg packs; matrix adds +2 */
static int compute_words(const Instruction *idef, AddrMode modes[], size_t nops){
    int words = 1;
    (void)idef; /* sizing here does not need the full idef */
    if (nops == 0) return words;

    if (nops == 2 && modes[0]==AM_REG && modes[1]==AM_REG)
        return words + 1; /* packed reg-reg */

    if (nops >= 1) {
        words += (modes[0]==AM_MAT) ? 2 : (modes[0]!=AM_INVALID ? 1 : 0);
    }
    if (nops >= 2) {
        words += (modes[1]==AM_MAT) ? 2 : (modes[1]!=AM_INVALID ? 1 : 0);
    }
    return words;
}

/* ---------- directives ---------- */

/* bind_label_at_dc — if label exists, attach DC (DATA) */
static void bind_label_at_dc(MemoryImage *mem, Symbol **symtab, ErrorList *errors, int line, const char *label_opt){
    if (label_opt && *label_opt) {
        Symbol *ex = find_symbol(*symtab, label_opt);
        if (ex) {
            if (ex->is_extern) add_err(errors,line,"label '%s' cannot redefine extern",label_opt);
            else if (ex->address != 0) add_err(errors,line,"duplicate label '%s'",label_opt);
            else { ex->address = mem->DC; ex->type = SYMBOL_DATA; }
        } else add_symbol(symtab, label_opt, mem->DC, SYMBOL_DATA);
    }
}

/* handle_data — parse unlimited comma-separated integers */
static void handle_data(MemoryImage *mem, ErrorList *errors, Symbol **symtab, int line,
                        const char *label_opt, char *args)
{
    char *p = args;
    char *endp;
    long v;

    if (label_opt && *label_opt) {
        Symbol *ex = find_symbol(*symtab, label_opt);
        if (ex) {
            if (ex->is_extern) add_error(errors, line, ".data: label redefines extern");
            else if (ex->address != 0) add_error(errors, line, ".data: duplicate label");
            else { ex->address = mem->DC; ex->type = SYMBOL_DATA; }
        } else {
            add_symbol(symtab, label_opt, mem->DC, SYMBOL_DATA);
        }
    }

    if (!args) { add_error(errors, line, ".data: missing numbers"); return; }

    for (;;) {
        char msg[160];

        /* skip leading spaces */
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '\0') break;  /* no more items */

        v = strtol(p, &endp, 10);
        if (endp == p) {
            /* build the message explicitly (add_error is not printf-style) */
            sprintf(msg, ".data: invalid integer near '%s'", p);
            add_error(errors, line, msg);
            return;
        }
        add_data_word(mem, (int)v);

        /* move past the parsed number and any spaces */
        p = endp;
        while (*p && isspace((unsigned char)*p)) p++;

        if (*p == ',') { p++; continue; }      /* next value */
        else if (*p == '\0') break;            /* end of list */
        else {
            add_error(errors, line, ".data: expected comma");
            return;
        }
    }
}

/* handle_string — emit bytes of quoted string (incl. NUL) */
static void handle_string(MemoryImage *mem, ErrorList *errors, Symbol **symtab, int line,
                          const char *label_opt, char *args){
    unsigned char *bytes = NULL;
    size_t n = 0, i;

    bind_label_at_dc(mem, symtab, errors, line, label_opt);

    if (!parse_string_literal(args, &bytes, &n)) { add_err(errors,line,".string: expected quoted string"); return; }
    for (i=0;i<n;i++) add_data_word(mem, (int)bytes[i]);
    free(bytes);
}

/* handle_extern — mark symbol as extern (create if needed) */
static void handle_extern(Symbol **symtab, ErrorList *errors, int line, char *args){
    char name[MAX_LABEL_LEN]={0};
    Symbol *existing;

    /* take first token as name */
    {
        int i=0;
        args = (char*)lstrip(args);
        while (args[i] && !isspace((unsigned char)args[i]) && i < MAX_LABEL_LEN-1) { name[i]=args[i]; i++; }
        name[i]='\0';
    }
    if (!name[0]) { add_err(errors,line,".extern: missing symbol name"); return; }
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

/* handle_entry — mark symbol as entry (create if needed) */
static void handle_entry(Symbol **symtab, ErrorList *errors, int line, char *args){
    char name[MAX_LABEL_LEN]={0};
    Symbol *sym;

    /* take first token as name */
    {
        int i=0;
        args = (char*)lstrip(args);
        while (args[i] && !isspace((unsigned char)args[i]) && i < MAX_LABEL_LEN-1) { name[i]=args[i]; i++; }
        name[i]='\0';
    }
    if (!name[0]) { add_err(errors,line,".entry: missing symbol name"); return; }
    if (!is_valid_label_name(name)) { add_err(errors,line,".entry: invalid name '%s'",name); return; }
    sym = find_symbol(*symtab, name);
    if (!sym) { add_symbol(symtab, name, 0, SYMBOL_CODE); sym = find_symbol(*symtab, name); }
    if (sym) sym->is_entry = 1;
}

/* handle_mat — parse .mat [R][C] + initializers (zero-fill) */
static void handle_mat(MemoryImage *mem, ErrorList *errors, Symbol **symtab, int line,
                       const char *label_opt, char *args){
    int R = 0, C = 0, total, i;
    char *p = args;
    char *vals_dup = NULL;
    char *parts[512];
    int n = 0;

    bind_label_at_dc(mem, symtab, errors, line, label_opt);

    p = lstrip(p);
    if (*p != '[') { add_err(errors,line,".mat: expected [rows][cols]"); return; }
    R = (int)strtol(p+1, &p, 10);
    if (*p != ']') { add_err(errors,line,".mat: malformed rows"); return; }
    p++;
    p = lstrip(p);
    if (*p != '[') { add_err(errors,line,".mat: expected [cols]"); return; }
    C = (int)strtol(p+1, &p, 10);
    if (*p != ']') { add_err(errors,line,".mat: malformed cols"); return; }
    p++;
    if (R <= 0 || C <= 0 || R > 64 || C > 64) { add_err(errors,line,".mat: invalid dimensions"); return; }
    total = R * C;

    /* optional initializer list after space/comma */
    p = (char*)lstrip(p);
    if (*p) {
        vals_dup = xstrdup(p);
        if (!vals_dup) { add_err(errors,line,"oom"); return; }
        n = split_commas_inplace(vals_dup, parts, 512);
    }

    for (i=0; i<total; ++i) {
        int v = 0;
        if (i < n) {
            char *tok = lstrip(parts[i]);
            char *endp = NULL;
            long lv;
            if (*tok=='\0') { add_err(errors,line,".mat: empty value"); v = 0; }
            else {
                lv = strtol(tok, &endp, 10);
                if (endp == tok || *lstrip(endp) != '\0') { add_err(errors,line,".mat: invalid integer '%s'", tok); v = 0; }
                else v = (int)lv;
            }
        }
        add_data_word(mem, v);
    }

    if (vals_dup) free(vals_dup);
}

/* ---------- instruction handling (sizing + label binding) ---------- */

/* handle_instruction — parse mnemonic/ops, size words, bind label */
static void handle_instruction(MemoryImage *mem, ErrorList *errors, Symbol **symtab, int line,
                               const char *label_opt, char *cursor)
{
    char mnemonic[16] = {0};
    const Instruction *idef;
    char *s, *opsbuf;
    char *ops[2] = {0, 0};
    int nops = 0, i;
    AddrMode modes[2] = { AM_INVALID, AM_INVALID };
    int words;

    if (!cursor) return;

    /* mnemonic */
    {
        char *p = cursor;
        int k=0;
        p = lstrip(p);
        while (*p && !isspace((unsigned char)*p) && *p!=',' && k < (int)sizeof(mnemonic)-1) {
            mnemonic[k++] = *p++;
        }
        mnemonic[k] = '\0';
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
        nops = split_commas_inplace(opsbuf, ops, 2);
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
        modes[i] = parse_operand_mode(ops[i]);
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

/* bump_data_symbols_by_icf — offset DATA labels by final code size+base */
static void bump_data_symbols_by_icf(Symbol *head, int icf_words){
    int bump = LOGICAL_BASE + icf_words;
    Symbol *s = head;
    for (; s; s = s->next) if (s->type==SYMBOL_DATA && !s->is_extern) s->address += bump;
}

/* ---------- entry point ---------- */

/* first_pass — scan file, fill symtab/DC, and compute IC */
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
        if (raw_len > MAX_LINE_LENGTH) add_err(errors,line_no,"line too long (> 80 chars)");

        strip_comment(linebuf);
        trim_inplace(linebuf);
        if (*linebuf == '\0') continue;

        cursor = linebuf;
        has_label = take_leading_label(&cursor, label);

        /* first token after (optional) label */
        {
            int i=0; char *p = (char*)lstrip(cursor);
            while (p[i] && !isspace((unsigned char)p[i]) && p[i] != ',' && i < (int)(sizeof(tok)-1)) { tok[i]=p[i]; i++; }
            tok[i]='\0';
        }
        if (!tok[0]) {
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
            else if (strcmp(tok,".extern")==0) {
                if (has_label) add_err(errors,line_no,"label before .extern is ignored");
                handle_extern(symtab,errors,line_no,cursor);
            } else if (strcmp(tok,".entry")==0) {
                if (has_label) add_err(errors,line_no,"label before .entry is ignored");
                handle_entry(symtab,errors,line_no,cursor);
            } else if (strcmp(tok,".mat")==0) {
                handle_mat(mem,errors,symtab,line_no,has_label?label:NULL,cursor);
            }
        } else {
            handle_instruction(mem,errors,symtab,line_no,has_label?label:NULL,cursor);
        }
    }

    fclose(fp);
    bump_data_symbols_by_icf(*symtab, mem->IC);
    return 1;
}

