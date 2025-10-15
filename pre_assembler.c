/* pre_assembler.c — ANSI C (C90)
   Minimal macro expander:
   mcro NAME
     <body lines...>
   endmcro   (also accepts "mcroend")
   Usage lines that are exactly "NAME" expand to the stored body.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "pre_assembler.h"
#include "error_list.h"

#define MAX_MACROS       64
#define MAX_MACRO_NAME   32
#define MAX_LINE_LEN     256

typedef struct {
    char **lines;     /* body lines */
    int    count;
    int    cap;
} LineBuf;

typedef struct {
    char    name[MAX_MACRO_NAME];
    LineBuf body;
} Macro;

static Macro g_macros[MAX_MACROS];
static int   g_macro_count = 0;

/* -------- small utils -------- */
static char *lstrip(char *s) { while (*s && isspace((unsigned char)*s)) s++; return s; }
static void rstrip_inplace(char *s) {
    size_t n = strlen(s);
    while (n && (s[n-1]=='\n' || s[n-1]=='\r' || isspace((unsigned char)s[n-1]))) s[--n] = '\0';
}
static void trim_inplace(char *s) { char *ls = lstrip(s); if (ls != s) memmove(s, ls, strlen(ls)+1); rstrip_inplace(s); }

static int is_valid_name(const char *name) {
    size_t i, n = strlen(name);
    if (n == 0 || n >= MAX_MACRO_NAME) return 0;
    if (!isalpha((unsigned char)name[0])) return 0;
    for (i = 1; i < n; i++) {
        if (!isalnum((unsigned char)name[i]) && name[i] != '_') return 0;
    }
    return 1;
}

static void lb_init(LineBuf *b) { b->lines = NULL; b->count = 0; b->cap = 0; }
static void lb_free(LineBuf *b) {
    int i;
    for (i = 0; i < b->count; i++) free(b->lines[i]);
    free(b->lines);
    b->lines = NULL; b->count = b->cap = 0;
}
static int lb_push(LineBuf *b, const char *line) {
    int newcap;
    char *copy;
    if (b->count == b->cap) {
        newcap = (b->cap == 0) ? 8 : b->cap * 2;
        b->lines = (char**)realloc(b->lines, newcap * sizeof(char*));
        if (!b->lines) return 0;
        b->cap = newcap;
    }
    copy = (char*)malloc(strlen(line) + 1);
    if (!copy) return 0;
    strcpy(copy, line);
    b->lines[b->count++] = copy;
    return 1;
}

static void macros_reset(void) {
    int i;
    for (i = 0; i < g_macro_count; i++) lb_free(&g_macros[i].body);
    g_macro_count = 0;
}

static int macro_index_by_name(const char *name) {
    int i;
    for (i = 0; i < g_macro_count; i++) {
        if (strcmp(g_macros[i].name, name) == 0) return i;
    }
    return -1;
}

/* Build "<base>.amx": strip the last extension (if any) before adding ".amx" */
static void make_out_path(const char *src, char *out, size_t out_sz) {
    const char *base = src, *p = src, *dot = NULL, *slash = NULL;
    size_t len;

    if (out_sz == 0) return;

    /* find last path separator and last dot after it */
    while (*p) { if (*p=='/' || *p=='\\') slash = p; p++; }
    if (slash) base = slash + 1;
    for (p = base; *p; ++p) if (*p == '.') dot = p;

    /* length of stem without extension */
    len = dot ? (size_t)(dot - base) : strlen(base);
    if (len >= out_sz) len = out_sz - 1;

    memcpy(out, base, len);
    out[len] = '\0';

    /* append ".amx" if room */
    if (strlen(out) + 4 < out_sz) strcat(out, ".amx");
}

/* -------- parsing & expansion -------- */

/* Pass 1: collect macros into g_macros; report duplicates, nested, missing end */
static int collect_macros(FILE *fp, ErrorList *errors) {
    char line[MAX_LINE_LEN + 4];
    int line_no = 0;
    int in_macro = 0;
    Macro *cur = NULL;

    while (fgets(line, sizeof(line), fp)) {
        char work[MAX_LINE_LEN + 4];
        char *p;
        line_no++;

        strcpy(work, line);
        trim_inplace(work);
        if (*work == '\0' || *work == ';') continue;

        p = work;
        if (!in_macro) {
            /* check for "mcro NAME" */
            if (strncmp(p, "mcro", 4) == 0 && (p[4] == '\0' || isspace((unsigned char)p[4]))) {
                char name[MAX_MACRO_NAME] = {0};
                p = lstrip(p + 4);
                if (*p == '\0') { add_error(errors, line_no, "mcro: missing name"); return 0; }
                /* read name token */
                {
                    size_t i = 0;
                    while (p[i] && !isspace((unsigned char)p[i]) && i < MAX_MACRO_NAME-1) {
                        name[i] = p[i]; i++;
                    }
                    name[i] = '\0';
                }
                if (!is_valid_name(name)) { add_error(errors, line_no, "mcro: invalid name"); return 0; }
                if (macro_index_by_name(name) >= 0) { add_error(errors, line_no, "mcro: duplicate name"); return 0; }
                if (g_macro_count >= MAX_MACROS) { add_error(errors, line_no, "too many macros"); return 0; }

                cur = &g_macros[g_macro_count++];
                strcpy(cur->name, name);
                lb_init(&cur->body);
                in_macro = 1;
                continue;
            }
            /* not a mcro line; ignore here */
        } else {
            /* inside macro body: look for endmcro / mcroend */
            if (strcmp(p, "endmcro") == 0 || strcmp(p, "mcroend") == 0) {
                in_macro = 0;
                cur = NULL;
                continue;
            }
            /* store raw line (as-is) */
            if (!lb_push(&g_macros[g_macro_count-1].body, line)) {
                add_error(errors, line_no, "out of memory");
                return 0;
            }
        }
    }

    if (in_macro) {
        add_error(errors, line_no, "unterminated macro (missing endmcro)");
        return 0;
    }
    return 1;
}

/* Expand: copy src->out, skipping macro definitions; replace lines that equal macro name */
static int expand_to(FILE *fp_in, FILE *fp_out) {
    char line[MAX_LINE_LEN + 4];

    while (fgets(line, sizeof(line), fp_in)) {
        char work[MAX_LINE_LEN + 4];
        char *p;
        int i;

        strcpy(work, line);
        trim_inplace(work);

        /* skip macro definition blocks entirely */
        if (*work && *work != ';') {
            p = work;
            if (strncmp(p, "mcro", 4) == 0 && (p[4] == '\0' || isspace((unsigned char)p[4]))) {
                /* skip until endmcro / mcroend */
                while (fgets(line, sizeof(line), fp_in)) {
                    strcpy(work, line);
                    trim_inplace(work);
                    if (strcmp(work, "endmcro") == 0 || strcmp(work, "mcroend") == 0) break;
                }
                continue; /* do not write mcro/endmcro to output */
            }
        }

        /* try macro substitution: line must be exactly a macro name (ignoring spaces) */
        for (i = 0; i < g_macro_count; i++) {
            if (strcmp(work, g_macros[i].name) == 0) {
                /* write body */
                int j;
                for (j = 0; j < g_macros[i].body.count; j++) {
                    fputs(g_macros[i].body.lines[j], fp_out);
                    /* ensure newline at end (preserve if already present) */
                    if (g_macros[i].body.lines[j][0] != '\0') {
                        size_t L = strlen(g_macros[i].body.lines[j]);
                        if (L == 0 || g_macros[i].body.lines[j][L-1] != '\n')
                            fputc('\n', fp_out);
                    } else {
                        fputc('\n', fp_out);
                    }
                }
                break; /* handled */
            }
        }
        if (i < g_macro_count) continue; /* was a macro call; already emitted body */

        /* otherwise, pass original line through unchanged */
        fputs(line, fp_out);
    }
    return 1;
}

/* -------- public API -------- */
int pre_assemble(const char *src_path,
                 char *out_path, size_t out_path_sz,
                 ErrorList *errors)
{
    FILE *fin = NULL, *fout = NULL;
    long pos;

    macros_reset();

    if (!src_path || !*src_path) { add_error(errors, 0, "pre_assemble: empty path"); return 0; }

    /* open input */
    fin = fopen(src_path, "r");
    if (!fin) { add_error(errors, 0, "pre_assemble: cannot open source"); return 0; }

    /* pass 1: collect macros */
    if (!collect_macros(fin, errors)) { fclose(fin); macros_reset(); return 0; }

    /* rewind and expand to .amx */
    pos = ftell(fin);
    (void)pos; /* silence unused in strict builds */
    rewind(fin);

    if (out_path && out_path_sz > 0) {
        make_out_path(src_path, out_path, out_path_sz);
    } else {
        char dummy[8]; (void)dummy;
    }

    {
        char pathbuf[512];
        const char *target;

        if (out_path && out_path_sz > 0) {
            target = out_path;
        } else {
            strcpy(pathbuf, "out.amx");
            target = pathbuf;
        }

        fout = fopen(target, "w");
        if (!fout) { fclose(fin); macros_reset(); add_error(errors, 0, "pre_assemble: cannot open output"); return 0; }

        if (!expand_to(fin, fout)) {
            fclose(fin); fclose(fout); macros_reset();
            add_error(errors, 0, "pre_assemble: expand failed");
            return 0;
        }
        fclose(fout);
    }

    fclose(fin);
    macros_reset();
    return 1;
}

