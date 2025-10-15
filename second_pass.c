#include <stdio.h>
#include <string.h>

#include "second_pass.h"
#include "first_pass.h"
#include "instruction_encoder.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

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

    /* reset IC/DC before running pass 1 (so code starts at 100, not 200) */
    init_memory_image(mem);

    /* Run pass 1 first to populate *symbols and data image */
    if (!first_pass(filename, symbols, mem, errors)) {
        add_error(errors, 0, "First pass failed");
        return 0;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        add_error(errors, 0, "Failed to open file for second pass");
        return 0;
    }

    /* Encode code from scratch in pass 2 */
    mem->IC = 0;
    mem->fixup_count = 0;
    memset(mem->code, 0, sizeof(mem->code));

    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        encode_instruction(line, *symbols, mem, errors, line_num);
    }
    fclose(fp);

    /* Resolve fixups */
    for (i = 0; i < mem->fixup_count; i++) {
        Fixup *fx = &mem->fixups[i];
        Symbol *sym = find_symbol(*symbols, fx->label);
        if (!sym) {
            sprintf(msg, "Undefined label: %s", fx->label);
            add_error(errors, fx->line, msg);
        } else {
            int idx = fx->address - LOGICAL_BASE;
            if (idx >= 0 && idx < MAX_CODE_SIZE) {
                int are = sym->is_extern ? ARE_E : ARE_R;
                mem->code[idx] = pack_value_word_fixup(sym->address, are);
            } else {
                sprintf(msg, "Patch address out of range for label: %s", fx->label);
                add_error(errors, fx->line, msg);
            }
        }
    }

    return 1;
}

