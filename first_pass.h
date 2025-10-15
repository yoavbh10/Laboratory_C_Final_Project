#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

/* 
 * Performs the first pass of the assembler
 * 
 * filename: expanded source file (.am)
 * symtab:   pointer to symbol table head pointer
 * mem:      memory image
 * errors:   error list
 * 
 * Returns 0 if completed (even if errors exist), non-zero if file cannot be read.
 */
int first_pass(const char *filename, Symbol **symtab, MemoryImage *mem, ErrorList *errors);

#endif

