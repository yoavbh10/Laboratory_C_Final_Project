#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "instruction_encoder.h"
#include "instruction_set.h"
#include "addressing_modes.h"
#include "error_list.h"
#include "memory_image.h"

/* ==================== HELPER FUNCTIONS ==================== */

static void trim_whitespace(char *str)
{
    char *end;
    char *start = str;

    while (*start && isspace((unsigned char)*start))
        start++;

    if (start != str)
        memmove(str, start, strlen(start) + 1);

    end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end))
        *end-- = '\0';
}

static char *copy_string(const char *src)
{
    char *dst;
    size_t len;
    if (!src)
        return NULL;
    len = strlen(src) + 1;
    dst = (char *)malloc(len);
    if (dst)
        strcpy(dst, src);
    return dst;
}

/* Detect addressing mode */
static AddressingType detect_addressing_mode(const char *operand)
{
    if (!operand) return ADDR_DIRECT;
    if (is_immediate(operand)) return ADDR_IMMEDIATE;
    if (is_register(operand)) return ADDR_REGISTER;
    /* matrix support can be added later */
    return ADDR_DIRECT;
}

/* Tokenize instruction line */
static int tokenize_instruction_line(const char *line,
                                     char *label_out,
                                     char *opcode_out,
                                     char *operands_out[2])
{
    char copy[MAX_LINE_LENGTH];
    char *token;
    char *rest;
    char *comma;
    int operand_count = 0;

    strncpy(copy, line, MAX_LINE_LENGTH - 1);
    copy[MAX_LINE_LENGTH - 1] = '\0';
    trim_whitespace(copy);

    label_out[0] = '\0';
    opcode_out[0] = '\0';
    operands_out[0] = operands_out[1] = NULL;

    if (copy[0] == '\0' || copy[0] == ';')
        return -1;

    token = strtok(copy, " \t");
    if (!token)
        return 0;

    /* optional label */
    if (token[strlen(token) - 1] == ':') {
        token[strlen(token) - 1] = '\0';
        strncpy(label_out, token, MAX_LINE_LENGTH - 1);
        token = strtok(NULL, " \t");
        if (!token)
            return 0;
    }

    strncpy(opcode_out, token, MAX_LINE_LENGTH - 1);

    rest = strtok(NULL, "");
    if (rest) {
        trim_whitespace(rest);
        comma = strchr(rest, ',');
        if (comma) {
            *comma = '\0';
            operands_out[0] = copy_string(rest);
            operands_out[1] = copy_string(comma + 1);
            if (operands_out[0]) trim_whitespace(operands_out[0]);
            if (operands_out[1]) trim_whitespace(operands_out[1]);
            operand_count = 2;
        } else if (*rest != '\0') {
            operands_out[0] = copy_string(rest);
            if (operands_out[0]) trim_whitespace(operands_out[0]);
            operand_count = 1;
        }
    }

    return operand_count;
}

/* Build instruction word with addressing modes */
static int build_instruction_word(const Instruction *instr,
                                  char *operands[2])
{
    int word = 0;
    AddressingType src_mode = ADDR_IMMEDIATE;
    AddressingType dst_mode = ADDR_IMMEDIATE;

    if (instr->operands == 2) {
        src_mode = detect_addressing_mode(operands[0]);
        dst_mode = detect_addressing_mode(operands[1]);
    } else if (instr->operands == 1) {
        dst_mode = detect_addressing_mode(operands[0]);
    }

    /* opcode bits 6–9 */
    word |= (instr->opcode & 0xF) << 6;
    /* dst addressing bits 4–5 */
    word |= (dst_mode & 0x3) << 2;
    /* src addressing bits 2–3 (note: swapped positions in doc) */
    word |= (src_mode & 0x3) << 4;
    /* A/R/E bits (set to Absolute = 00) */
    word |= 0;

    return word;
}

/* ==================== ENCODE INSTRUCTION ==================== */
int encode_instruction(const char *line,
                       Symbol *symbols,
                       MemoryImage *mem,
                       ErrorList *errors,
                       int line_num)
{
    char label[MAX_LINE_LENGTH];
    char opcode[MAX_LINE_LENGTH];
    char *operands[2];
    const Instruction *instr;
    int operand_count;
    int i;

    (void)symbols; /* currently unused */

    operand_count = tokenize_instruction_line(line, label, opcode, operands);

    if (operand_count == -1)
        return 1; /* empty or comment */

    if (opcode[0] == '\0') {
        add_error(errors, line_num, "Missing opcode");
        return 0;
    }

    instr = find_instruction(opcode);
    if (!instr) {
        char msg[128];
        sprintf(msg, "Unknown opcode: %s", opcode);
        add_error(errors, line_num, msg);
        for (i = 0; i < operand_count; i++)
            if (operands[i]) free(operands[i]);
        return 0;
    }

    if (operand_count != instr->operands) {
        char msg[128];
        sprintf(msg, "Operand count mismatch for %s (expected %d, got %d)",
                opcode, instr->operands, operand_count);
        add_error(errors, line_num, msg);
        for (i = 0; i < operand_count; i++)
            if (operands[i]) free(operands[i]);
        return 0;
    }

    /* --- Addressing mode validation --- */
    if (instr->operands == 2) {
        AddressingType src_mode = detect_addressing_mode(operands[0]);
        AddressingType dst_mode = detect_addressing_mode(operands[1]);
        if (!instr->allowed_src[src_mode]) {
            char msg[128];
            sprintf(msg, "Invalid addressing mode for source operand: %s", operands[0]);
            add_error(errors, line_num, msg);
        }
        if (!instr->allowed_dst[dst_mode]) {
            char msg[128];
            sprintf(msg, "Invalid addressing mode for destination operand: %s", operands[1]);
            add_error(errors, line_num, msg);
        }
    } else if (instr->operands == 1) {
        AddressingType dst_mode = detect_addressing_mode(operands[0]);
        if (!instr->allowed_dst[dst_mode]) {
            char msg[128];
            sprintf(msg, "Invalid addressing mode for operand: %s", operands[0]);
            add_error(errors, line_num, msg);
        }
    }

    /* --- Build first instruction word --- */
    add_code_word(mem, build_instruction_word(instr, operands));

    /* --- Add placeholder words for operands (will be improved later) --- */
    for (i = 0; i < operand_count; i++)
        add_code_word(mem, 0);

    for (i = 0; i < operand_count; i++)
        if (operands[i]) free(operands[i]);

    return 1;
}

