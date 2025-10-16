/* second_pass.c
 * Pass 2: encode instructions into code image, resolve symbols,
 * patch extern/internal refs, and write output files (.ob/.ent/.ext).
 */

#include <stdio.h>
#include <string.h>

#include "second_pass.h"
#include "instruction_encoder.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"
#include "output_files.h"

#ifndef ARE_A
#define ARE_A 0
#endif
#ifndef ARE_E
#define ARE_E 1
#endif
#ifndef ARE_R
#define ARE_R 2
#endif

#define LOGICAL_BASE       100
#define MAX_LINE_LENGTH     80

/* strip_comment/lstrip/rstrip/trim — simple string cleanup */
static void strip_comment(char *s){ for(;*s;++s){ if(*s==';'){*s='\0';break;} } }
static char *lstrip(char *s){ while(*s && (*s==' '||*s=='\t')) ++s; return s; }
static void rstrip_inplace(char *s){
    size_t n = strlen(s);
    while(n && (s[n-1]=='\n'||s[n-1]=='\r'||s[n-1]==' '||s[n-1]=='\t')) s[--n]='\0';
}
static void trim_inplace(char *s){ char *p=lstrip(s); if(p!=s) memmove(s,p,strlen(p)+1); rstrip_inplace(s); }

/* encode_instructions_from_file — re-encode instructions into mem->code */
static int encode_instructions_from_file(const char *expanded_path,
                                         Symbol *symbols,
                                         MemoryImage *mem,
                                         ErrorList *errors)
{
    FILE *fp = fopen(expanded_path, "r");
    char linebuf[1024]; int line_no = 0;

    if (!fp) { add_error(errors, 0, "cannot open expanded source for pass-2"); return 0; }

    mem->IC = 0;
    mem->fixup_count = 0;

    while (fgets(linebuf, sizeof(linebuf), fp)) {
        size_t raw_len; char work[1024];
        line_no++;

        raw_len = strcspn(linebuf, "\r\n");
        if (raw_len > MAX_LINE_LENGTH) add_error(errors, line_no, "line too long (> 80 chars)");

        strcpy(work, linebuf);
        strip_comment(work);
        trim_inplace(work);
        if (*work == '\0') continue;

        /* encode instruction (still collect errors if any) */
        (void)encode_instruction(work, symbols, mem, errors, line_no);
    }

    fclose(fp);
    return 1;
}

/* second_pass — resolve fixups and write outputs */
int second_pass(const char *expanded_filename, Symbol **symbols, MemoryImage *mem, ErrorList *errors)
{
    int k, had_errors = 0;

    if (!expanded_filename || !symbols || !*symbols || !mem || !errors) {
        add_error(errors, 0, "second_pass: invalid arguments");
        return 0;
    }

    of_init(); /* reset extern-use list */

    /* 1) encode instructions */
    if (!encode_instructions_from_file(expanded_filename, *symbols, mem, errors)) return 0;

    /* 2) resolve fixups */
    for (k = 0; k < mem->fixup_count; ++k) {
        Fixup *fx = &mem->fixups[k];
        const Symbol *sym = find_symbol(*symbols, fx->label);
        int idx = fx->word_index;
        int abs_addr = LOGICAL_BASE + idx; /* used in .ext file */
        int are_bits, patched;

        if (!sym) {
            char msg[256];
            sprintf(msg, "Undefined label: %s", fx->label);
            add_error(errors, fx->line, msg);
            had_errors = 1;
            continue;
        }

        if (sym->is_extern) {
            are_bits = ARE_E;
            of_record_extern_use(sym->name, abs_addr);
        } else {
            are_bits = ARE_R;
        }

        if (idx >= 0 && idx < MAX_CODE_SIZE) {
            /* patch code word: high 8 bits = value, low 2 bits = ARE */
            int value8 = (sym->address & 0xFF);
            patched = (value8 << 2) | (are_bits & 0x3);
            mem->code[idx] = patched;
        }
    }

    if (had_errors) return 0;

    /* 3) write output files */
    write_output_files(expanded_filename, mem, *symbols);
    return 1;
}

