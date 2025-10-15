#include <stdio.h>
#include "first_pass.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

int main() {
    Symbol *symbols = NULL;
    MemoryImage mem;
    ErrorList errors;

    init_error_list(&errors);
    init_memory_image(&mem);

    if (!first_pass("test_entry_extern.am", &symbols, &mem, &errors)) {
        printf("First pass failed.\n");
        print_and_clear_errors(&errors);
        return 1;
    }

    printf("First pass completed.\n");
    print_symbol_table(symbols);
    print_and_clear_errors(&errors);

    free_symbol_table(&symbols);
    return 0;
}

