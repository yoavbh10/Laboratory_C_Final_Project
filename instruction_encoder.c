#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "instruction_encoder.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"
#include "instruction_set.h"

static int is_register(const char *token) {
    return token[0] == 'r' && isdigit((unsigned char)token[1]) &&
           token[2] == '\0' && token[1] >= '0' && token[1] <= '7';
}

static int addressing_mode(const char *token) {
    if (token[0] == '#') return 0;    /* immediate */
    if (is_register(token)) return 2; /* register direct */
    return 1;                         /* direct (label) */
}

int encode_instruction(const char *line, Symbol *symbols,
                       MemoryImage *mem, ErrorList *errors)
{
    char op[10], src[32], dst[32];
    int opcode, mode_src, mode_dst;
    int words_added = 1; /* at least one word for instruction */

    /* parse: opcode src , dst */
    int n = sscanf(line, "%s %[^,], %s", op, src, dst);
    if (n < 2) {
        add_error(errors, 0, "Invalid instruction syntax");
        return 0;
    }

    /* lookup opcode */
    opcode = get_opcode(op);
    if (opcode < 0) {
        add_error(errors, 0, "Unknown opcode");
        return 0;
    }

    /* determine addressing modes */
    mode_src = addressing_mode(src);
    mode_dst = (n == 3) ? addressing_mode(dst) : addressing_mode(src);

    /* encode first word */
    {
        int word = (opcode << 12) | (mode_src << 10) | (mode_dst << 8);
        add_code_word(mem, word);
    }

    /* handle operands */
    if (mode_src == 0) { /* immediate */
        int val = atoi(src + 1);
        add_code_word(mem, val);
        words_added++;
    } else if (mode_src == 1) { /* label */
        Symbol *sym = find_symbol(symbols, src);
        if (!sym) {
            add_error(errors, 0, "Undefined label in source operand");
        } else {
            add_code_word(mem, sym->address);
        }
        words_added++;
    }

    if (n == 3) { /* destination operand provided */
        if (mode_dst == 0) {
            int val = atoi(dst + 1);
            add_code_word(mem, val);
            words_added++;
        } else if (mode_dst == 1) {
            Symbol *sym = find_symbol(symbols, dst);
            if (!sym) {
                add_error(errors, 0, "Undefined label in destination operand");
            } else {
                add_code_word(mem, sym->address);
            }
            words_added++;
        }
    }

    return words_added;
}

