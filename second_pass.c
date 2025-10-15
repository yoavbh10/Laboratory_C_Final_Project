#include <stdio.h>
#include <string.h>

#include "second_pass.h"
#include "instruction_encoder.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

int second_pass(const char *filename, Symbol *symbols,
                MemoryImage *mem, ErrorList *errors) {
    FILE *fp;
    char line[256];
    int line_num = 0;

    fp = fopen(filename, "r");
    if (!fp) {
        add_error(errors, 0, "Failed to open file for second pass");
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        encode_instruction(line, line_num, symbols, mem, errors);
    }

    fclose(fp);
    return 1;
}

