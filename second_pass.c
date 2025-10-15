#include <stdio.h>
#include <string.h>

#include "second_pass.h"
#include "first_pass.h"
#include "instruction_encoder.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"
#include "output_files.h"

#define LOGICAL_BASE 100
enum { ARE_A = 0, ARE_E = 1, ARE_R = 2 };

/* Pack a value into [9..2] with ARE in [1..0] (ANSI C) */
static int pack_value_word_fixup(int value, int are){
    return ((value & 0xFF) << 2) | (are & 0x3);
}

int second_pass(const char *filename, Symbol **symbols,
                MemoryImage *mem, ErrorList *errors)
{
    FILE *fp;
    char line[MAX_LINE_LENGTH+4];
    int line_num = 0;
    int i;
    char msg[128];

    /* per-run init */
    init_memory_image(mem);   /* IC/DC as counts */
	of_reset();               /* clear extern-use list */

    /* Run pass 1: build symbols + data, compute IC as word count */
    if (!first_pass(filename, symbols, mem, errors)) {
        add_error(errors, 0, "First pass failed");
        return 0;
    }

    /* Now encode code words from source */
    fp = fopen(filename, "r");
    if (!fp) {
        add_error(errors, 0, "Failed to open file for second pass");
        return 0;
    }

    mem->IC = 0;               /* rebuild code image from scratch */
    mem->fixup_count = 0;
    memset(mem->code, 0, sizeof(mem->code));

    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        encode_instruction(line, *symbols, mem, errors, line_num);
    }
    fclose(fp);

    /* Resolve fixups: patch code words with real addresses, record extern uses */
    for (i = 0; i < mem->fixup_count; i++) {
        Fixup *fx = &mem->fixups[i];
        Symbol *sym = find_symbol(*symbols, fx->label);
        if (!sym) {
            sprintf(msg, "Undefined label: %s", fx->label);
            add_error(errors, fx->line, msg);
        } else {
            int idx = fx->address - LOGICAL_BASE; /* fixup address is logical */
            if (idx >= 0 && idx < MAX_CODE_SIZE) {
                int are = sym->is_extern ? ARE_E : ARE_R;
                mem->code[idx] = pack_value_word_fixup(sym->address, are);
                if (sym->is_extern) {
                    /* For .ext: record each use occurrence */
                    of_record_extern_use(sym->name, fx->address);
                }
            } else {
                sprintf(msg, "Patch address out of range for label: %s", fx->label);
                add_error(errors, fx->line, msg);
            }
        }
    }

    /* Finally, write output files (.ob / .ent / .ext) */
    write_output_files(filename, mem, *symbols);

    return 1;
}

