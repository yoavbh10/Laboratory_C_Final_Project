#ifndef SECOND_PASS_H
#define SECOND_PASS_H

#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

#ifndef MAX_LINE_LENGTH
#define MAX_LINE_LENGTH 80
#endif

/* Perform second pass:
   - runs first pass to populate *symbols and data image
   - encodes code and records fixups
   - resolves fixups using *symbols */
int second_pass(const char *filename, Symbol **symbols,
                MemoryImage *mem, ErrorList *errors);

#endif

