#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "first_pass.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

/* Track entry symbols for second pass */
#define MAX_ENTRY_COUNT 50
static char entry_symbols[MAX_ENTRY_COUNT][MAX_LABEL_LEN];
static int entry_count = 0;

/* Handle .data directive */
static void handle_data(MemoryImage *mem, const char *params)
{
    char *copy;
    char *token;
    char *endptr;
    long val;

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

        /* --- Directive / Instruction handling --- */
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
            char extern_name[MAX_LABEL_LEN];
            sscanf(p + 7, "%s", extern_name);
            if (extern_name[0] != '\0') {
                if (!add_symbol(symtab, extern_name, 0, SYMBOL_CODE)) {
				    add_error(errors, line_num, "Failed to add external symbol");
				} else {
				    Symbol *ext = find_symbol(*symtab, extern_name);
				    if (ext) ext->is_extern = 1;
				}
            } else {
                add_error(errors, line_num, "Missing symbol name after .extern");
            }

        } else if (strncmp(p, ".entry", 6) == 0) {
            /* entry symbol (stored for second pass) */
            char entry_name[MAX_LABEL_LEN];
            sscanf(p + 6, "%s", entry_name);
            if (entry_name[0] != '\0') {
                if (entry_count < MAX_ENTRY_COUNT) {
                    strncpy(entry_symbols[entry_count], entry_name, MAX_LABEL_LEN - 1);
                    entry_symbols[entry_count][MAX_LABEL_LEN - 1] = '\0';
                    entry_count++;
                } else {
                    add_error(errors, line_num, "Too many entry symbols");
                }
            } else {
                add_error(errors, line_num, "Missing symbol name after .entry");
            }

        } else {
            /* instruction placeholder */
            if (label[0])
                add_symbol(symtab, label, mem->IC, SYMBOL_CODE);
            add_code_word(mem, 0); /* placeholder instruction */
        }
    }

    fclose(fp);
    return 1;
}

