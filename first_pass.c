#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "first_pass.h"

static int extract_label(const char *line, char *label)
{
    const char *colon = strchr(line, ':');
    int len;
    if (!colon)
        return 0;
    len = (int)(colon - line);
    if (len <= 0 || len >= MAX_LABEL_LEN)
        return -1;
    strncpy(label, line, len);
    label[len] = '\0';
    return 1;
}

static void handle_data(const char *params, MemoryImage *mem)
{
	char *copy = (char*)malloc(strlen(params) + 1);
	char *token = strtok(copy, ",");
	
	if (copy) strcpy(copy, params);
    while (token)
    {
        while (isspace((unsigned char)*token))
            token++;
        add_data_word(mem, atoi(token));
        token = strtok(NULL, ",");
    }
    free(copy);
}

static void handle_string(const char *param, MemoryImage *mem)
{
    const char *p = strchr(param, '"');
    const char *q = strrchr(param, '"');
    if (!p || !q || p == q)
        return;
    for (p++; p < q; p++)
        add_data_word(mem, (int)(unsigned char)(*p));
    add_data_word(mem, 0);
}

int first_pass(const char *filename, Symbol **symtab,
               MemoryImage *mem, ErrorList *errors)
{
    FILE *fp = fopen(filename, "r");
    char line[MAX_LINE_LENGTH];
    int line_num = 0;

    if (!fp)
    {
        add_error(errors, 0, "Cannot open source file");
        return 0;
    }

    while (fgets(line, sizeof(line), fp))
    {
        char *p = line;
        char label[MAX_LABEL_LEN] = {0};
        line_num++;

        while (isspace((unsigned char)*p))
            p++;

        if (*p == ';' || *p == '\0' || *p == '\n')
            continue;

        if (extract_label(p, label) == 1)
        {
            add_symbol(symtab, label, mem->IC, SYMBOL_CODE);
            p = strchr(p, ':') + 1;
        }

        if (strncmp(p, ".data", 5) == 0)
        {
            handle_data(p + 5, mem);
        }
        else if (strncmp(p, ".string", 7) == 0)
        {
            handle_string(p + 7, mem);
        }
        else
        {
            /* stub instruction encoding */
            add_code_word(mem, 0);
            add_code_word(mem, 0);
            add_code_word(mem, 0);
        }
    }

    fclose(fp);
    return 1;
}

