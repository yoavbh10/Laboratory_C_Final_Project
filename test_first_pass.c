#include "first_pass.h"
#include <stdio.h>

int main(void) {
    Symbol *symtab = NULL; /* head of linked list */
    MemoryImage mem;
    ErrorList errors;

    init_memory_image(&mem);
    init_error_list(&errors);

    if (first_pass("test_macros.am", &symtab, &mem, &errors) == 0) {
        printf("First pass completed.\n");
        if (has_errors(&errors)) {
            printf("Errors found:\n");
            print_and_clear_errors(&errors);
        }
        printf("Symbol Table:\n");
        print_symbol_table(symtab);
        printf("\nMemory Image:\n");
        print_memory_image(&mem);
    } else {
        printf("Failed to read source file.\n");
    }

    free_symbol_table(symtab);
    free_error_list(&errors);

    return 0;
}

