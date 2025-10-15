#include "output_files.h"
#include <stdio.h>
#include <string.h>

/* ---- extern-use collector ---------------------------------------------- */

typedef struct {
    char name[MAX_LABEL_LEN];
    int  address;                 /* absolute address (decimal) */
} ExtUse;

#define MAX_EXT_USES 1024

static ExtUse g_ext_uses[MAX_EXT_USES];
static int    g_ext_use_count = 0;

void of_init(void)
{
    g_ext_use_count = 0;
}

void of_record_extern_use(const char *name, int use_address)
{
    if (!name) return;
    if (g_ext_use_count >= MAX_EXT_USES) return;
    strncpy(g_ext_uses[g_ext_use_count].name, name, MAX_LABEL_LEN - 1);
    g_ext_uses[g_ext_use_count].name[MAX_LABEL_LEN - 1] = '\0';
    g_ext_uses[g_ext_use_count].address = use_address;
    g_ext_use_count++;
}

/* ---- base-4 "abcd" helpers --------------------------------------------- */

static void to_base4_letters(unsigned int n, int width, char *out)
{
    static const char digits[4] = {'a','b','c','d'};
    int i;
    for (i = width - 1; i >= 0; --i) {
        out[i] = digits[n & 3u];
        n >>= 2;
    }
    out[width] = '\0';
}

static void addr_to_b4(int addr, char *out4)
{
    unsigned int v = (unsigned int)(addr & 0xFF); /* 0..255 */
    to_base4_letters(v, 4, out4);
}

static void word_to_b4(int word, char *out5)
{
    unsigned int v = (unsigned int)(word & 0x3FF); /* 10-bit word */
    to_base4_letters(v, 5, out5);
}

/* ---- path helper -------------------------------------------------------- */

static void derive_base_name(const char *src_filename, char *dst, size_t dstsz)
{
    const char *last_slash = NULL, *p = src_filename, *dot = NULL;
    size_t len;

    while (*p) { if (*p == '/' || *p == '\\') last_slash = p; p++; }
    if (last_slash) src_filename = last_slash + 1;

    for (p = src_filename; *p; ++p) if (*p == '.') dot = p;

    if (dot && dot > src_filename) len = (size_t)(dot - src_filename);
    else                           len = strlen(src_filename);

    if (len >= dstsz) len = dstsz - 1;
    memcpy(dst, src_filename, len);
    dst[len] = '\0';
}

/* ---- writers ------------------------------------------------------------ */

void write_output_files(const char *src_filename,
                        const MemoryImage *mem,
                        const Symbol *symbols)
{
    char base[256];
    char path_ob[300], path_ent[300], path_ext[300];
    FILE *fob, *fent, *fext;
    int i, wrote_ent = 0, wrote_ext = 0;
    int code_size, data_size;

    if (!src_filename || !mem) return;

    derive_base_name(src_filename, base, sizeof(base));

    /* Build paths (ANSI C: use sprintf) */
    sprintf(path_ob,  "%s.ob",  base);
    sprintf(path_ent, "%s.ent", base);
    sprintf(path_ext, "%s.ext", base);

    /* Sizes */
    code_size = mem->IC;
    if (code_size < 0) code_size = 0;
    if (code_size > MAX_CODE_SIZE) code_size = MAX_CODE_SIZE;

    data_size = mem->DC;
    if (data_size < 0) data_size = 0;
    if (data_size > MAX_DATA_SIZE) data_size = MAX_DATA_SIZE;

    /* .ob */
    fob = fopen(path_ob, "w");
    if (!fob) return;

    /* header as base-4 letters (5 digits each to be safe) */
    {
        char b4_code[6], b4_data[6];
        to_base4_letters((unsigned int)code_size, 5, b4_code);
        to_base4_letters((unsigned int)data_size, 5, b4_data);
        fprintf(fob, "%s %s\n", b4_code, b4_data);
    }

    /* code words at 100..IC-1 */
    for (i = 0; i < code_size; ++i) {
        int abs_addr = 100 + i;
        char a4[5], w5[6];
        addr_to_b4(abs_addr, a4);
        word_to_b4(mem->code[i], w5);
        fprintf(fob, "%s %s\n", a4, w5);
    }

    /* data words follow code */
    for (i = 0; i < data_size; ++i) {
        int abs_addr = 100 + code_size + i;
        char a4[5], w5[6];
        addr_to_b4(abs_addr, a4);
        word_to_b4(mem->data[i], w5);
        fprintf(fob, "%s %s\n", a4, w5);
    }

    fclose(fob);

    /* .ent (only entries that are not extern) */
    fent = fopen(path_ent, "w");
    if (fent) {
        const Symbol *s = symbols;
        while (s) {
            if (s->is_entry && !s->is_extern) {
                char a4[5];
                addr_to_b4(s->address, a4);
                fprintf(fent, "%s %s\n", s->name, a4);
                wrote_ent = 1;
            }
            s = s->next;
        }
        fclose(fent);
        if (!wrote_ent) remove(path_ent);
    }

    /* .ext (each recorded extern use) */
    if (g_ext_use_count > 0) {
        fext = fopen(path_ext, "w");
        if (fext) {
            for (i = 0; i < g_ext_use_count; ++i) {
                char a4[5];
                addr_to_b4(g_ext_uses[i].address, a4);
                fprintf(fext, "%s %s\n", g_ext_uses[i].name, a4);
            }
            fclose(fext);
            wrote_ext = 1;
        }
    }
    if (!wrote_ext) remove(path_ext);
}

