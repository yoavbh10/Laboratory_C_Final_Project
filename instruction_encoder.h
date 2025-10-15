#ifndef INSTRUCTION_ENCODER_H
#define INSTRUCTION_ENCODER_H

#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

/* Encodes a single instruction line into machine code.
   For now, this is just a stub that returns success. */
int encode_instruction(const char *line, Symbol *symbols, MemoryImage *mem, ErrorList *errors);

#endif

