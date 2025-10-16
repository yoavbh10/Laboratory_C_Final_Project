/* second_pass.h
 * Re-encodes instructions, resolves fixups with symbol table,
 * and writes output files (.ob/.ent/.ext).
 */
 
#ifndef SECOND_PASS_H
#define SECOND_PASS_H

#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

#ifndef MAX_LINE_LENGTH
#define MAX_LINE_LENGTH 80
#endif

/* second_pass — encode code, resolve symbols, write outputs */
int second_pass(const char *filename, Symbol **symbols,
                MemoryImage *mem, ErrorList *errors);

#endif /* SECOND_PASS_H */

