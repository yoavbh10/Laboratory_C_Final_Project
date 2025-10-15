#include "symbol_table.h"
#include <stdio.h>

int main(void) {
    Symbol *table = NULL;

    add_symbol(&table, "START", 100, SYMBOL_CODE);
    add_symbol(&table, "DATA1", 200, SYMBOL_DATA);
    add_symbol(&table, "EXT1",  0,   SYMBOL_EXTERNAL);

    printf("Before update:\n");
    print_symbol_table(table);

    update_data_symbols(table, 100);

    printf("\nAfter update:\n");
    print_symbol_table(table);

    free_symbol_table(table);
    return 0;
}

