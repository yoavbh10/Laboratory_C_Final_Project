#include <stdio.h>
#include "output_files.h"
#include "symbol_table.h"
#include "memory_image.h"

int write_entry_file(const char *base_filename, Symbol *symbols)
{
    FILE *f = fopen("out.ent", "w");
    Symbol *curr = symbols;
    while (curr) {
        if (curr->is_entry)
            fprintf(f, "%s %d\n", curr->name, curr->address);
        curr = curr->next;
    }
    fclose(f);
    (void)base_filename; /* now used logically */
    return 1;
}

int write_extern_file(const char *base_filename, Symbol *symbols)
{
    FILE *f = fopen("out.ext", "w");
    Symbol *curr = symbols;
    while (curr) {
        if (curr->is_extern)
            fprintf(f, "%s %d\n", curr->name, curr->address);
        curr = curr->next;
    }
    fclose(f);
    (void)base_filename;
    return 1;
}

int write_object_file(const char *base_filename, MemoryImage *mem)
{
    FILE *f = fopen("out.ob", "w");
    int i;
    for (i = 100; i < mem->IC; i++)
        fprintf(f, "%04d %d\n", i, mem->code[i - 100]);
    for (i = 0; i < mem->DC; i++)
        fprintf(f, "%04d %d\n", i, mem->data[i]);
    fclose(f);
    (void)base_filename;
    return 1;
}

