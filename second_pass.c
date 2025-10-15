/* second_pass.c — ANSI C (C90)
   Pass 2: encode code image from the expanded file, resolve label fixups,
   record extern uses, and write output files.
*/
#include <string.h>
#include <stdio.h>

#include "second_pass.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"
#include "output_files.h"
#include "instruction_encoder.h"

/* Fallback ARE codes if not visible via other headers */
#ifndef ARE_A
#define ARE_A 0
#endif
#ifndef ARE_E
#define ARE_E 1
#endif
#ifndef ARE_R
#define ARE_R 2
#endif

/* Logical base address for code */
#ifndef LOGICAL_BASE
#define LOGICAL_BASE 100
#endif

int second_pass(const char *filename, Symbol **symbols, MemoryImage *mem, ErrorList *errors)
{
    FILE *fp;
    char linebuf[1024];
    int line_no = 0;
    int had_errors = 0;
    int k;

    if (!filename || !symbols || !*symbols || !mem || !errors) return 0;

    /* fresh extern-use list */
    of_init();

    /* Rebuild the code image from the expanded file: reset IC and fixups */
    mem->IC = 0;
    mem->fixup_count = 0;

    fp = fopen(filename, "r");
    if (!fp) {
        char msg[256];
        sprintf(msg, "cannot open source '%s'", filename);
        add_error(errors, 0, msg);
        return 0;
    }

    /* Encode instructions & collect fixups (data directives are ignored here) */
    while (fgets(linebuf, sizeof(linebuf), fp)) {
        line_no++;
        /* encode_instruction appends code words & fixups; reports errors via errors */
        (void)encode_instruction(linebuf, *symbols, mem, errors, line_no);
    }
    fclose(fp);

    /* Resolve all recorded fixups (label references) */
    for (k = 0; k < mem->fixup_count; ++k) {
        Fixup *fx = &mem->fixups[k];
        const Symbol *sym = find_symbol(*symbols, fx->label);
        int are_bits;
        int idx;

        if (!sym) {
            char msg[256];
            sprintf(msg, "Undefined label: %s", fx->label);
            add_error(errors, fx->line, msg);
            had_errors = 1;
            continue;
        }

        if (sym->is_extern) {
            are_bits = ARE_E;               /* external reference */
            of_record_extern_use(sym->name, fx->address);
        } else {
            are_bits = ARE_R;               /* relocatable */
        }

        /* Patch the word in the code image at absolute address fx->address */
        idx = fx->address - LOGICAL_BASE;
        if (idx >= 0 && idx < MAX_CODE_SIZE) {
            int value10 = sym->address & 0x3FF;   /* 10-bit payload */
            mem->code[idx] = (value10 & ~0x3) | (are_bits & 0x3);
        }
    }

    if (had_errors) return 0;

    /* Emit output files (.ob/.ent/.ext) now that fixups are resolved */
    write_output_files(filename, mem, *symbols);
    return 1;
}

