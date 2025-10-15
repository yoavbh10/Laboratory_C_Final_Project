#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

/* Max source line length per spec (not counting '\n') */
#ifndef MAX_LINE_LENGTH
#define MAX_LINE_LENGTH 80
#endif

/* Run pass-1 on an already macro-expanded source file.
   - Builds/updates the symbol table (labels, .extern, .entry flags)
   - Sizes the code image (IC) and fully emits the data image (DC) for
     .data / .string / .mat
   - Validates instruction forms and addressing to keep IC consistent
   Returns 1 on success (even if nonfatal warnings), 0 on fatal I/O error. */
int first_pass(const char *filename, Symbol **symtab, MemoryImage *mem, ErrorList *errors);

#endif /* FIRST_PASS_H */

