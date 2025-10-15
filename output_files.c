#include "output_files.h"
#include "memory_image.h"
#include "symbol_table.h"
#include <stdio.h>
#include <string.h>

/* ===== simple store for extern uses collected during fixups ===== */
typedef struct {
    char name[MAX_LABEL_LEN];
    int  addr; /* logical code address of the use site */
} ExtUse;

#define MAX_EXT_USES 512
static ExtUse g_ext_uses[MAX_EXT_USES];
static int    g_ext_use_count = 0;

void of_reset(void) {
    g_ext_use_count = 0;
}

void of_record_extern_use(const char *name, int addr) {
    if (!name || !*name) return;
    if (g_ext_use_count >= MAX_EXT_USES) return;
    strncpy(g_ext_uses[g_ext_use_count].name, name, MAX_LABEL_LEN - 1);
    g_ext_uses[g_ext_use_count].name[MAX_LABEL_LEN - 1] = '\0';
    g_ext_uses[g_ext_use_count].addr = addr;
    g_ext_use_count++;
}

/* ===== helpers ===== */

/* derive a basename from input path and strip .amx and/or .am suffixes */
static void derive_base(const char *in, char *base, size_t n) {
    const char *p = in, *start = in;
    size_t len;

    if (n == 0) return;
    base[0] = '\0';

    /* strip directory part */
    while (*p) {
        if (*p == '/' || *p == '\\') start = p + 1;
        p++;
    }
    strncpy(base, start, n - 1);
    base[n - 1] = '\0';

    /* strip suffixes in sequence: first .amx, then .am if still present */
    len = strlen(base);
    if (len >= 4 && strcmp(base + len - 4, ".amx") == 0) {
        base[len - 4] = '\0';
        len -= 4;
    }
    if (len >= 3 && strcmp(base + len - 3, ".am") == 0) {
        base[len - 3] = '\0';
    }
}

/* write a 10-bit word as unsigned decimal (mask to 10 bits) */
static void fprint_word10(FILE *f, int w) {
    int v = w & 0x3FF;
    fprintf(f, "%d\n", v);
}

/* ===== public API ===== */

void write_output_files(const char *input_filename,
                        const MemoryImage *mem,
                        const Symbol *symbols)
{
    char base[512];
    char path_ob[600], path_ent[600], path_ext[600];
    FILE *fob, *f;
    const Symbol *s;
    int i;

    if (!input_filename || !*input_filename || !mem) return;

    /* base name without .am/.amx */
    derive_base(input_filename, base, sizeof(base));

    /* compose paths */
    sprintf(path_ob,  "%s.ob",  base);
    sprintf(path_ent, "%s.ent", base);
    sprintf(path_ext, "%s.ext", base);

    /* ---- .ob ---- */
    fob = fopen(path_ob, "w");
    if (fob) {
        int code_words = mem->IC - 100; /* logical addresses 100..IC-1 */
        int data_words = mem->DC;

        fprintf(fob, "%d %d\n", code_words, data_words);

        /* code section: logical 100..IC-1 */
        for (i = 100; i < mem->IC; i++) {
            fprint_word10(fob, mem->code[i - 100]);
        }
        /* data section: 0..DC-1 */
        for (i = 0; i < mem->DC; i++) {
            fprint_word10(fob, mem->data[i]);
        }
        fclose(fob);
    }

    /* ---- .ent (only if at least one entry & not extern) ---- */
    {
        int wrote_any = 0;
        f = fopen(path_ent, "w");
        if (f) {
            for (s = symbols; s; s = s->next) {
                if (s->is_entry && !s->is_extern) {
                    fprintf(f, "%s %d\n", s->name, s->address);
                    wrote_any = 1;
                }
            }
            fclose(f);
            if (!wrote_any) {
                /* no entries -> remove empty file */
                remove(path_ent);
            }
        }
    }

    /* ---- .ext (one line per extern use recorded) ---- */
    if (g_ext_use_count > 0) {
        f = fopen(path_ext, "w");
        if (f) {
            for (i = 0; i < g_ext_use_count; i++) {
                fprintf(f, "%s %d\n", g_ext_uses[i].name, g_ext_uses[i].addr);
            }
            fclose(f);
        }
    } else {
        /* make sure no stale file remains */
        remove(path_ext);
    }
}

