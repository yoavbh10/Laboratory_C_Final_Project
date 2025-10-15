#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "first_pass.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

/* Handle .data directive */
static void handle_data(MemoryImage *mem, const char *params)
{
    char *copy;
    char *token;
    char *endptr;
    long val;

    /* make a copy of params because strtok modifies it */
    copy = (char *)malloc(strlen(params) + 1);
    if (!copy)
        return;
    strcpy(copy, params);

    token = strtok(copy, ",");
    while (token != NULL) {
        while (isspace((unsigned char)*token))
            token++;
        val = strtol(token, &endptr, 10);
        if (*endptr == '\0' || isspace((unsigned char)*endptr))
            add_data_word(mem, (int)val);
        token = strtok(NULL, ",");
    }
    free(copy);
}

/* Handle .string directive */
static void handle_string(MemoryImage *mem, const char *params)
{
    const char *p = params;
    if (*p == '\"')
        p++;
    while (*p && *p != '\"')
        add_data_word(mem, (int)(unsigned char)*p++);
    add_data_word(mem, 0); /* Null terminator */
}

int first_pass(const char *filename, Symbol **symtab, MemoryImage *mem, ErrorList *errors)
{
    FILE *fp;
    char line[MAX_LINE_LENGTH];
    int line_num = 0;

    fp = fopen(filename, "r");
    if (!fp) {
        add_error(errors, 0, "Cannot open source file");
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        char *p = line;
        char label[MAX_LABEL_LEN] = {0};

        line_num++;
        while (isspace((unsigned char)*p)) p++;
        if (*p == ';' || *p == '\0')
            continue; /* comment or empty */

        /* label check */
        if (strchr(p, ':')) {
            char *colon = strchr(p, ':');
            int len = (int)(colon - p);
            if (len > 0 && len < MAX_LABEL_LEN) {
                strncpy(label, p, len);
                label[len] = '\0';
                p = colon + 1;
                while (isspace((unsigned char)*p)) p++;
            }
        }

        if (strncmp(p, ".data", 5) == 0) {
            if (label[0])
                add_symbol(symtab, label, mem->DC, SYMBOL_DATA);
            handle_data(mem, p + 5);
        } else if (strncmp(p, ".string", 7) == 0) {
            if (label[0])
                add_symbol(symtab, label, mem->DC, SYMBOL_DATA);
            handle_string(mem, p + 7);
        } else if (strncmp(p, ".extern", 7) == 0) {
            /* extern symbol */
        } else if (strncmp(p, ".entry", 6) == 0) {
            /* entry symbol, handled in second pass */
        } else {
            /* instruction placeholder */
            if (label[0])
                add_symbol(symtab, label, mem->IC, SYMBOL_CODE);
            add_code_word(mem, 0); /* placeholder */
        }
    }

    fclose(fp);
    return 1;
}

