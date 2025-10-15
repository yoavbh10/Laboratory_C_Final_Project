#include <stdio.h>
#include <string.h>

#include "pre_assembler.h"
#include "first_pass.h"
#include "second_pass.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"
#include "output_files.h"

int main(int argc, char **argv) {
    int i;
    if (argc < 2) {
        fprintf(stderr, "usage: %s <source1.as> [source2.as ...]\n", argv[0]);
        return 1;
    }

    for (i = 1; i < argc; ++i) {
        const char *src = argv[i];
        char expanded_path[512] = {0};
        MemoryImage mem;
        ErrorList errors;
        Symbol *symbols = NULL;
        int ok;

        init_error_list(&errors);
        init_memory_image(&mem);
        init_symbol_table(&symbols);

        /* Pre-assemble (macro expansion) */
        if (!pre_assemble(src, expanded_path, sizeof(expanded_path), &errors)) {
            print_and_clear_errors(&errors);
            free_symbol_table(&symbols);
            continue;
        }
        printf("[pre] macros expanded -> %s\n", expanded_path[0] ? expanded_path : src);

        /* Pass 1 */
        printf("[pass1] running on: %s\n", expanded_path[0] ? expanded_path : src);
        ok = first_pass(expanded_path[0] ? expanded_path : src, &symbols, &mem, &errors);
        if (!ok) {
            print_and_clear_errors(&errors);
            free_symbol_table(&symbols);
            if (expanded_path[0]) remove(expanded_path); /* cleanup temp */
            continue;
        }
        printf("[pass1] OK (IC=%d, DC=%d)\n", mem.IC, mem.DC);

        /* Pass 2 + write outputs */
        printf("[pass2] resolving fixups & writing outputs for base: %s\n", src);
        ok = second_pass(src, &symbols, &mem, &errors);
        if (!ok) {
            print_and_clear_errors(&errors);
            free_symbol_table(&symbols);
            if (expanded_path[0]) remove(expanded_path); /* cleanup temp */
            continue;
        }

        print_and_clear_errors(&errors);
        free_symbol_table(&symbols);
        if (expanded_path[0]) remove(expanded_path); /* SUCCESS: cleanup temp */
        printf("[done]\n");
    }

    return 0;
}

