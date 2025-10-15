#ifndef SECOND_PASS_H
#define SECOND_PASS_H

#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

#define MAX_LINE_LENGTH 256  /* consistent with first pass */

/* Perform second pass: 
   - resolves addresses for operands 
   - handles .entry 
   - generates .ob, .ent, .ext */
int second_pass(const char *filename, Symbol *symbols, MemoryImage *mem, ErrorList *errors);

#endif

