#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "instruction_encoder.h"
#include "instruction_set.h"
#include "addressing_modes.h"
#include "symbol_table.h"
#include "error_list.h"
#include "memory_image.h"

/* Trim leading/trailing whitespace */
static void trim(char *str) {
    char *start = str;
    char *end;

    while (isspace((unsigned char)*start)) start++;
    if (start != str) memmove(str, start, strlen(start) + 1);

    end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
}

/* Tokenize line into tokens array */
static int tokenize(const char *line, char tokens[][31], int max_tokens) {
    int count = 0;
    const char *p = line;

    while (*p && count < max_tokens) {
        while (isspace((unsigned char)*p) || *p == ',') p++;
        if (!*p) break;

        {
            int len = 0;
            while (*p && !isspace((unsigned char)*p) && *p != ',' && len < 30) {
                tokens[count][len++] = *p++;
            }
            tokens[count][len] = '\0';
            count++;
        }
    }
    return count;
}

int encode_instruction(const char *line,
                       int line_num,
                       Symbol *symbols,
                       MemoryImage *mem,
                       ErrorList *errors) {
    char tokens[4][31];
    char temp[256];
    int token_count;
    char *opcode;
    int start_index;
    const Instruction *instr;
    int operand_count;

    strncpy(temp, line, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    trim(temp);

    /* Skip empty/comment */
    if (temp[0] == '\0' || temp[0] == ';')
        return 1;

    token_count = tokenize(temp, tokens, 4);
    if (token_count == 0) return 1;

    opcode = tokens[0];
    start_index = 1;

    /* If line starts with a label */
    if (strchr(opcode, ':')) {
        if (token_count < 2) {
            add_error(errors, line_num, "Label without instruction");
            return 0;
        }
        opcode = tokens[1];
        start_index = 2;
    }

    instr = find_instruction(opcode);
    if (!instr) {
        add_error(errors, line_num, "Unknown opcode");
        return 0;
    }

    operand_count = token_count - start_index;
    if (operand_count != instr->operands) {
        char msg[80];
        sprintf(msg, "Operand count mismatch for %s (expected %d, got %d)",
                opcode, instr->operands, operand_count);
        add_error(errors, line_num, msg);
        return 0;
    }

    /* --- Minimal encoding: just add opcode word for now --- */
    add_code_word(mem, instr->opcode); /* <-- confirm function name */

    return 1;
}

