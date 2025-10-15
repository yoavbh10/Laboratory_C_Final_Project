#ifndef INSTRUCTION_ENCODER_H
#define INSTRUCTION_ENCODER_H

#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

#define MAX_LINE_LENGTH 256  /* unified with second_pass.h */

/* Encode a single instruction line into machine code */
int encode_instruction(const char *line, Symbol *symbols, MemoryImage *mem, ErrorList *errors, int line_num);

#endif

