#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "second_pass.h"
#include "first_pass.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

/* Simple pretty-printer to match your prior output style */
static const char* type_str(SymbolType t){ return (t==SYMBOL_CODE) ? "CODE" : "DATA"; }

int main(int argc, char **argv) {
    const char *fname;
    Symbol *symbols = NULL;
    MemoryImage mem;
    ErrorList errors;

    if (argc < 2) {
        printf("Usage: %s <file.am>\n", argv[0]);
        return 1;
    }
    fname = argv[1];

    init_memory_image(&mem);
    init_error_list(&errors);

    if (second_pass(fname, &symbols, &mem, &errors)) {
        printf("Second pass completed successfully.\n");
    }

    /* Print symbols (for test visibility) */
    {
        Symbol *p = symbols;
        while (p) {
            printf("Symbol: %-32s Address:%6d  Type:%-5s Entry:%d Extern:%d\n",
                   p->name, p->address, type_str(p->type), p->is_entry, p->is_extern);
            p = p->next;
        }
    }

    /* Print and clear any errors */
    print_and_clear_errors(&errors);

    /* Cleanup */
    free_symbol_table(&symbols);

    return 0;
}

