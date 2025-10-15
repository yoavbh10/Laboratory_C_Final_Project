/* first_pass.h
 * Scans source file, builds symbol table and data image.
 * Also computes code size (IC).
 */
 
#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

#define MAX_LINE_LENGTH 80  /* spec: max line length */

/* first_pass — scans source, builds symbols & sizes code/data */
int first_pass(const char *filename, Symbol **symtab, MemoryImage *mem, ErrorList *errors);

#endif /* FIRST_PASS_H */

