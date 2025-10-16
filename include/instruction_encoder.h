/* instruction_encoder.h
 * Encodes assembly instructions into machine code words.
 * Used during second pass to fill memory image.
 */
 
#ifndef INSTRUCTION_ENCODER_H
#define INSTRUCTION_ENCODER_H

#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

#ifndef MAX_LINE_LENGTH
#define MAX_LINE_LENGTH 80
#endif

#define MAX_TOKENS 10

/* encode_instruction — encodes one assembly line into memory image */
int encode_instruction(const char *line,
                       Symbol *symbols,
                       MemoryImage *mem,
                       ErrorList *errors,
                       int line_num);

#endif /* INSTRUCTION_ENCODER_H */

