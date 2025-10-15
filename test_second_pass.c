#include <stdio.h>
#include <stdlib.h>
#include "second_pass.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

/* Updated: Accepts an optional filename argument. */
int main(int argc, char *argv[])
{
    const char *filename = "test_second_pass.am"; /* default */
    Symbol *symbols = NULL;
    MemoryImage mem;
    ErrorList errors;

    init_error_list(&errors);

    if (argc > 1) {
        filename = argv[1];
    }

    init_memory_image(&mem);

    /* Add some test symbols for demonstration */
    add_symbol(&symbols, "MAIN", 100, SYMBOL_CODE);
    add_symbol(&symbols, "EXT_LABEL", 0, SYMBOL_CODE);

    /* Run second pass */
    if (!second_pass(filename, symbols, &mem, &errors)) {
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

