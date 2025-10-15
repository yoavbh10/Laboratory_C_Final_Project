#include <stdio.h>
#include <string.h>

#include "second_pass.h"
#include "instruction_encoder.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

int second_pass(const char *filename, Symbol *symbols,
                MemoryImage *mem, ErrorList *errors) {
    FILE *fp;
    char line[256];
    int line_num = 0;

    int i;              /* for fixup resolution loop */
    char msg[128];      /* for error formatting */

    fp = fopen(filename, "r");
    if (!fp) {
        add_error(errors, 0, "Failed to open file for second pass");
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        /* Encode each line; encoder will push opcode and create fixups for labels */
	    encode_instruction(line, symbols, mem, errors, line_num);
    }

    fclose(fp);

    /* Resolve recorded fixups: patch code words with real addresses */
    for (i = 0; i < mem->fixup_count; i++) {
        Fixup *fx = &mem->fixups[i];
        Symbol *sym = find_symbol(symbols, fx->label);
        if (!sym) {
            sprintf(msg, "Undefined label: %s", fx->label);
            add_error(errors, fx->line, msg);
        } else {
            int idx = fx->address - 100; /* convert absolute address to code[] index */
            if (idx >= 0 && idx < MAX_CODE_SIZE) {
                mem->code[idx] = sym->address;
            } else {
                /* Out-of-range patch location; report but continue */
                sprintf(msg, "Patch address out of range for label: %s", fx->label);
                add_error(errors, fx->line, msg);
            }
        }
    }

    return 1;
}

