#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "instruction_encoder.h"
#include "instruction_set.h"
#include "addressing_modes.h"
#include "error_list.h"
#include "memory_image.h"

#define MAX_OPERANDS 3 /* prevent overflow */

/* Convert string to lowercase in-place */
static void to_lower_str(char *s)
{
    while (*s) {
        *s = (char)tolower((unsigned char)*s);
        s++;
    }
}

/* Trim leading and trailing spaces */
static void trim_spaces(char *s)
{
    char *end;
    while (isspace((unsigned char)*s)) s++;
    if (*s == 0) return; /* empty string */
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
}

/* Check if line is comment or blank */
static int is_comment_or_blank(const char *line)
{
    const char *p = line;
    while (*p && isspace((unsigned char)*p)) p++;
    return (*p == ';' || *p == '\0');
}

/* Check if token is assembler directive */
static int is_directive(const char *token)
{
    return (token[0] == '.' &&
            (!strcmp(token, ".entry") ||
             !strcmp(token, ".extern") ||
             !strcmp(token, ".data") ||
             !strcmp(token, ".string") ||
             !strcmp(token, ".mat")));
}

/* Split instruction line into label(optional), opcode, operands */
static int tokenize_instruction_line(const char *line,
                                     char *label,
                                     char *opcode,
                                     char operands[MAX_OPERANDS][MAX_LINE_LENGTH],
                                     int *operand_count)
{
    char temp[MAX_LINE_LENGTH];
    char *p, *token;
    int count = 0;

    strncpy(temp, line, MAX_LINE_LENGTH - 1);
    temp[MAX_LINE_LENGTH - 1] = '\0';

    trim_spaces(temp);
    if (is_comment_or_blank(temp))
        return 0;

    p = temp;

    /* Check for label at start (ends with ':') */
    token = strtok(p, " \t");
    if (!token) return 0;

    if (token[strlen(token) - 1] == ':') {
        token[strlen(token) - 1] = '\0';
        strcpy(label, token);
        token = strtok(NULL, " \t");
        if (!token) {
            opcode[0] = '\0';
            *operand_count = 0;
            return 1;
        }
    } else {
        label[0] = '\0';
    }

    strcpy(opcode, token);
    to_lower_str(opcode);

    /* Collect operands (comma separated) */
    while ((token = strtok(NULL, ", \t")) != NULL && count < MAX_OPERANDS) {
        trim_spaces(token);
        strcpy(operands[count], token);
        count++;
    }
    *operand_count = count;
    return 1;
}

/* Detect addressing type */
static int detect_addressing_mode(const char *operand)
{
    if (is_immediate(operand)) return ADDR_IMMEDIATE;
    if (is_register(operand)) return ADDR_REGISTER;
    if (is_label(operand)) return ADDR_DIRECT;
    return -1; /* invalid */
}

/* Build instruction word (dummy for now) */
static int build_instruction_word(const Instruction *instr,
                                  int operand_count,
                                  char operands[MAX_OPERANDS][MAX_LINE_LENGTH],
                                  int *word)
{
    AddressingType src = ADDR_IMMEDIATE, dst = ADDR_IMMEDIATE;
    *word = instr->opcode << 6;

    if (operand_count == 2) {
        src = (AddressingType)detect_addressing_mode(operands[0]);
        dst = (AddressingType)detect_addressing_mode(operands[1]);
        if ((int)src == -1 || (int)dst == -1) return 0;
    } else if (operand_count == 1) {
        dst = (AddressingType)detect_addressing_mode(operands[0]);
        if ((int)dst == -1) return 0;
    }
    return 1;
}

/* Encode instruction line */
int encode_instruction(const char *line,
                       Symbol *symbols, /* currently unused */
                       MemoryImage *mem,
                       ErrorList *errors,
                       int line_num)
{
    char label[MAX_LINE_LENGTH] = "";
    char opcode[MAX_LINE_LENGTH] = "";
    char operands[MAX_OPERANDS][MAX_LINE_LENGTH];
    int operand_count = 0;
    const Instruction *instr;
    int word;
    char msg[256];

    (void)symbols; /* silence unused parameter warning */

    if (!tokenize_instruction_line(line, label, opcode, operands, &operand_count))
        return 1;

    if (is_directive(opcode))
        return 1;

    instr = find_instruction(opcode);
    if (!instr) {
        sprintf(msg, "Unknown opcode: %s", opcode);
        add_error(errors, line_num, msg);
        return 0;
    }

    if (operand_count != instr->operands) {
        sprintf(msg, "Operand count mismatch for %s (expected %d, got %d)",
                opcode, instr->operands, operand_count);
        add_error(errors, line_num, msg);
        return 0;
    }

    if ((strcmp(opcode, "stop") == 0 || strcmp(opcode, "rts") == 0) &&
        operand_count != 0) {
        sprintf(msg, "%s does not take operands", opcode);
        add_error(errors, line_num, msg);
        return 0;
    }

    if (!build_instruction_word(instr, operand_count, operands, &word)) {
        sprintf(msg, "Invalid addressing mode for operand(s)");
        add_error(errors, line_num, msg);
        return 0;
    }

    add_code_word(mem, word);
    return 1;
}

