#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "instruction_encoder.h"
#include "instruction_set.h"
#include "addressing_modes.h"
#include "error_list.h"

#define MAX_TOKENS 10

/* --- Helpers --- */

/* Trim whitespace in place */
static void trim(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
}

/* Remove comment starting with ';' */
static void strip_comment(char *line) {
    char *comment = strchr(line, ';');
    if (comment) *comment = '\0';
}

/* Remove leading label (ends with ':') */
static void strip_label(char *line) {
    char *colon = strchr(line, ':');
    if (colon) {
        memmove(line, colon + 1, strlen(colon + 1) + 1);
    }
}

/* Tokenize line into tokens[] and return count */
static int tokenize(const char *line, char *tokens[], int max_tokens) {
    char *copy;
    char *p;
    int count = 0;

    copy = (char *)malloc(strlen(line) + 1);
    if (!copy) return 0;
    strcpy(copy, line);

    p = strtok(copy, " \t,");
    while (p && count < max_tokens) {
        tokens[count] = (char *)malloc(strlen(p) + 1);
        strcpy(tokens[count], p);
        count++;
        p = strtok(NULL, " \t,");
    }
    free(copy);
    return count;
}

static void free_tokens(char *tokens[], int count) {
    int i;
    for (i = 0; i < count; i++)
        free(tokens[i]);
}

/* --- Main Encoding Function --- */
int encode_instruction(const char *line, Symbol *symbols, MemoryImage *mem, ErrorList *errors, int line_num) {
    char clean[MAX_LINE_LENGTH];
    char *tokens[MAX_TOKENS];
    int token_count;
    const Instruction *inst;

    /* Copy and preprocess line */
    strcpy(clean, line);
    strip_comment(clean);
    strip_label(clean);
    trim(clean);
    if (strlen(clean) == 0)
        return 1; /* empty line after stripping label & comment */

    token_count = tokenize(clean, tokens, MAX_TOKENS);
    if (token_count == 0) {
        free_tokens(tokens, token_count);
        add_error(errors, line_num, "Invalid instruction syntax");
        return 0;
    }

    inst = find_instruction(tokens[0]);
    if (!inst) {
        char msg[100];
        sprintf(msg, "Unknown opcode: %s", tokens[0]);
        add_error(errors, line_num, msg);
        free_tokens(tokens, token_count);
        return 0;
    }

    if (token_count - 1 != inst->operands) {
        char msg[100];
        sprintf(msg, "Operand count mismatch for %s", tokens[0]);
        add_error(errors, line_num, msg);
        free_tokens(tokens, token_count);
        return 0;
    }

    /* For now, just create placeholder code words */
    add_code_word(mem, inst->opcode << 6); /* simple encoding placeholder */

    free_tokens(tokens, token_count);
    return 1;
}

