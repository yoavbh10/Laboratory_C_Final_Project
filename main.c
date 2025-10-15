#include <stdio.h>
#include <string.h>

#include "pre_assembler.h"
#include "first_pass.h"
#include "second_pass.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

int main(int argc, char **argv) {
    const char *src;
    char expanded[512];
    MemoryImage mem;
    ErrorList errors;
    Symbol *symbols = NULL;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <source.am>\n", argv[0]);
        return 1;
    }
    src = argv[1];

    init_error_list(&errors);
    init_memory_image(&mem);

    /* Pre-assemble (macro expansion) to <src>.amx */
    expanded[0] = '\0';
    if (!pre_assemble(src, expanded, sizeof(expanded), &errors)) {
        /* Show errors from pre-assembler and exit */
        print_and_clear_errors(&errors);
        return 1;
    }

    /* Run passes on the expanded file */
    if (!second_pass(expanded, &symbols, &mem, &errors)) {
        print_and_clear_errors(&errors);
        free_symbol_table(&symbols);
        /* Optionally: remove(expanded); */
        return 1;
    }

    /* If we’re here, output files were already written by second_pass */
    print_and_clear_errors(&errors);
    free_symbol_table(&symbols);

    /* Optionally remove the expanded temp file after successful assembly */
    /* remove(expanded); */

    return 0;
}

