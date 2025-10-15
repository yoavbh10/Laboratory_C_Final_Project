#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

/* Spec line length */
#define MAX_LINE_LENGTH 80

int first_pass(const char *filename, Symbol **symtab, MemoryImage *mem, ErrorList *errors);

#endif

