#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pre_assembler.h"
#include "first_pass.h"
#include "second_pass.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

/* Helper: pick expanded or original, but always use ORIGINAL name
   for output files so you get foo.ob (not foo.amx.ob). */
static const char *choose_input_for_passes(const char *orig, const char *amx_path, int expanded) {
    (void)orig;
    return expanded ? amx_path : orig;
}

int main(int argc, char **argv) {
    const char *src;
    char amx_path[512];
    int expanded = 0;
    const char *pass_input;

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

    /* 1) Pre-assembler (macro expansion) */
    expanded = pre_assemble(src, amx_path, sizeof(amx_path), &errors);
    if (expanded < 0) {
        /* fatal error in macro stage */
        print_and_clear_errors(&errors);
        return 1;
    }
    if (expanded > 0) {
        printf("[pre] macros expanded -> %s\n", amx_path);
    } else {
        printf("[pre] no macros found; using original source\n");
    }

    pass_input = choose_input_for_passes(src, amx_path, expanded);

    /* 2) First pass */
    printf("[pass1] running on: %s\n", pass_input);
    if (!first_pass(pass_input, &symbols, &mem, &errors)) {
        printf("[pass1] FAILED\n");
        print_and_clear_errors(&errors);
        free_symbol_table(&symbols);
        return 1;
    }
    printf("[pass1] OK (IC=%d, DC=%d)\n", mem.IC, mem.DC);

    /* 3) Second pass + output write.
       IMPORTANT: we pass the ORIGINAL src name to second_pass so
       output_files.c derives base names like 'file.ob' (not 'file.amx.ob'). */
    printf("[pass2] resolving fixups & writing outputs for base: %s\n", src);
    if (!second_pass(src, &symbols, &mem, &errors)) {
        printf("[pass2] FAILED\n");
        print_and_clear_errors(&errors);
        free_symbol_table(&symbols);
        return 1;
    }

    print_and_clear_errors(&errors);
    free_symbol_table(&symbols);
    printf("[done]\n");
    return 0;
}

