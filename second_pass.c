#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "second_pass.h"
#include "instruction_encoder.h"
#include "output_files.h"
#include "symbol_table.h"
#include "error_list.h"

int second_pass(const char *filename, Symbol *symbols, MemoryImage *mem, ErrorList *errors)
{
    FILE *fp;
    char line[MAX_LINE_LENGTH];
    int line_num = 0;

    fp = fopen(filename, "r");
    if (!fp) {
        add_error(errors, 0, "Cannot open source file for second pass");
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        char *p = line;
        line_num++;

        while (isspace((unsigned char)*p)) p++;
        if (*p == ';' || *p == '\0')
            continue;

        if (strncmp(p, ".entry", 6) == 0) {
            char label[MAX_LABEL_LEN];
            if (sscanf(p + 6, "%s", label) == 1) {
                Symbol *sym = find_symbol(symbols, label);
                if (sym)
                    sym->is_entry = 1;
                else
                    add_error(errors, line_num, "Entry symbol not found");
            }
        }
        else if (strncmp(p, ".extern", 7) == 0) {
            /* already handled in first pass */
            continue;
        }
        else {
            if (!encode_instruction(p, symbols, mem, errors, line_num)) {
                char msg[64];
                sprintf(msg, "Instruction encoding failed at line %d", line_num);
                add_error(errors, line_num, msg);
            }
        }
    }

    fclose(fp);

    /* Write output files */
    write_entry_file(filename, symbols);
    write_extern_file(filename, symbols);
    write_object_file(filename, mem);

    return 1;
}

