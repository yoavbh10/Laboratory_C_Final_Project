#include <stdio.h>
#include "second_pass.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

int main(void)
{
    Symbol *symbols = NULL;
    MemoryImage mem;
    ErrorList errors;

    init_error_list(&errors);
    init_memory_image(&mem);

    /* Pre-populate symbol table like first pass would */
    add_symbol(&symbols, "MAIN", 100, SYMBOL_CODE);
    add_symbol(&symbols, "EXT_LABEL", 0, SYMBOL_CODE);
    symbols->is_extern = 1;  /* Mark EXT_LABEL as external */

    if (!second_pass("test_second_pass_input.am", symbols, &mem, &errors)) {
        printf("Second pass failed.\n");
        print_and_clear_errors(&errors);
        free_symbol_table(&symbols);
        return 1;
    }

    printf("Second pass completed successfully.\n");
    print_symbol_table(symbols);
    print_and_clear_errors(&errors);

    free_symbol_table(&symbols);
    return 0;
}

