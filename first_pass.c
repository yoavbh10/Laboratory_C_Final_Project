#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "first_pass.h"
#include "instruction_set.h"

/* Helpers remain the same */

int first_pass(const char *filename, Symbol **symtab, MemoryImage *mem, ErrorList *errors) {
    FILE *fp;
    char line[120];
    int line_num = 0;
    char label[32];

    fp = fopen(filename, "r");
    if (!fp) {
        printf("Error: Cannot open file %s\n", filename);
        return 1;
    }

    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        /* (skip comments/blank) */
        if (line[0] == ';' || line[0] == '\n')
            continue;

        /* Label check (very primitive) */
        if (strchr(line, ':')) {
            sscanf(line, "%31[^:]:", label);
            if (add_symbol(symtab, label, mem->IC, 0) != 0)
                add_error(errors, line_num, "Duplicate label definition");
        }

        /* Stub: treat as an instruction placeholder */
        add_code_word(mem, 0);
    }

    fclose(fp);
    return 0;
}

