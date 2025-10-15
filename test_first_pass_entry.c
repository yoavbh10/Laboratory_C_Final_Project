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

    if (!first_pass("test_entry.am", &symbols, &mem, &errors)) {
        printf("First pass failed.\n");
        print_and_clear_errors(&errors);
        return 1;
    }

    printf("First pass completed.\n");
    print_symbol_table(symbols);
    printf("Code Image (IC=%d):\n", mem.IC);
    {
        int i;
        for (i = 100; i < mem.IC; i++)
            printf("  [%d] %d\n", i, mem.code[i - 100]);
    }
    printf("Data Image (DC=%d):\n", mem.DC);
    {
        int i;
        for (i = 0; i < mem.DC; i++)
            printf("  [%d] %d\n", i, mem.data[i]);
    }

    print_and_clear_errors(&errors);
    free_symbol_table(&symbols);

    return 0;
}

