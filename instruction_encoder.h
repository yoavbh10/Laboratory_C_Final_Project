#ifndef INSTRUCTION_ENCODER_H
#define INSTRUCTION_ENCODER_H

#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

#define MAX_TOKENS 10
#define MAX_LINE_LENGTH 256  /* match second_pass.h for consistency */

/* Encodes one instruction line into memory image.
   Returns 1 on success, 0 on error. */
int encode_instruction(const char *line,
                       Symbol *symbols,
                       MemoryImage *mem,
                       ErrorList *errors,
                       int line_num);

#endif

