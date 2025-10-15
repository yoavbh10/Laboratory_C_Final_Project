#ifndef INSTRUCTION_ENCODER_H
#define INSTRUCTION_ENCODER_H

#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

#ifndef MAX_LINE_LENGTH
#define MAX_LINE_LENGTH 80
#endif

#define MAX_TOKENS 10

/* Encodes one instruction line into the memory image.
   Returns 1 on success, 0 on error.
   Safe to call on non-instruction lines (returns 1 and does nothing). */
int encode_instruction(const char *line,
                       Symbol *symbols,
                       MemoryImage *mem,
                       ErrorList *errors,
                       int line_num);

#endif

