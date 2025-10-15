#ifndef OUTPUT_FILES_H
#define OUTPUT_FILES_H

#include "symbol_table.h"
#include "memory_image.h"

int write_entry_file(const char *base_filename, Symbol *symbols);
int write_extern_file(const char *base_filename, Symbol *symbols);
int write_object_file(const char *base_filename, MemoryImage *mem);

#endif

