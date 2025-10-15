/* main.c
 * Entry point for the assembler. Handles args, runs pre-assembler, pass1 & pass2.
 * Cleans up temp files and checks memory limits.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "pre_assembler.h"
#include "first_pass.h"
#include "second_pass.h"
#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

#define LOGICAL_BASE 100

/* print_errors — minimal error printer if detailed struct is hidden */
static void print_errors(const ErrorList *list, const char *filename) {
    (void)list;
    fprintf(stderr, "[error] issues detected while processing %s (see messages above)\n", filename);
}

/* derive_source_name — ensure input ends with .as */
static void derive_source_name(const char *arg, char *out, size_t outsz) {
    const char *dot = NULL, *p = arg;
    size_t n = strlen(arg);
    if (n >= outsz) n = outsz - 1;
    memcpy(out, arg, n); out[n] = '\0';
    for (p = out; *p; ++p) if (*p == '.') dot = p;
    if (!dot) {
        if (strlen(out) + 3 < outsz) strcat(out, ".as");
    }
}

/* main — loops over files, runs assembler passes */
int main(int argc, char **argv)
{
    int i, ok_all = 1;
    if (argc < 2) {
        printf("usage: %s <file> [file ...]\n", argv[0]);
        return 0;
    }

    for (i = 1; i < argc; ++i) {
        char src_path[512];
        char expanded_am[512] = {0};
        ErrorList errors;
        Symbol *symbols = NULL;
        MemoryImage mem;

        derive_source_name(argv[i], src_path, sizeof(src_path));

        init_error_list(&errors);
        init_symbol_table(&symbols);
        init_memory_image(&mem);

        /* pre-assembler -> .am file */
        if (!pre_assemble(src_path, expanded_am, sizeof(expanded_am), &errors)) {
            print_errors(&errors, src_path);
            free_symbol_table(&symbols);
            ok_all = 0;
            continue;
        }

        /* first pass — builds symbol table & instruction skeletons */
        if (!first_pass(expanded_am[0] ? expanded_am : src_path, &symbols, &mem, &errors)) {
            print_errors(&errors, src_path);
            free_symbol_table(&symbols);
            if (expanded_am[0]) remove(expanded_am);
            ok_all = 0;
            continue;
        }

        /* memory must not exceed 255 */
        if (LOGICAL_BASE + mem.IC + mem.DC > 256) {
            add_error(&errors, 0, "memory overflow: code+data exceed address 255");
            print_errors(&errors, src_path);
            free_symbol_table(&symbols);
            if (expanded_am[0]) remove(expanded_am);
            ok_all = 0;
            continue;
        }

        /* second pass — resolves symbols & writes outputs */
        if (!second_pass(src_path, &symbols, &mem, &errors)) {
            print_errors(&errors, src_path);
            ok_all = 0;
        }

        free_symbol_table(&symbols);
        if (expanded_am[0]) remove(expanded_am);
    }

    return ok_all ? 0 : 1;
}

