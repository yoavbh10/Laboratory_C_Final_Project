#ifndef INSTRUCTION_ENCODER_H
#define INSTRUCTION_ENCODER_H

#include "symbol_table.h"
#include "memory_image.h"
#include "error_list.h"

/**
 * Encode a single assembly instruction line.
 *
 * @param line       The source line containing an instruction (already preprocessed, labels resolved)
 * @param line_num   Current source line number (for error reporting)
 * @param symbols    Symbol table (for resolving labels)
 * @param mem        Memory image (output encoded machine code)
 * @param errors     Error list for reporting encoding problems
 * @return 1 if successful, 0 if errors were added
 */
int encode_instruction(const char *line,
                       int line_num,
                       Symbol *symbols,
                       MemoryImage *mem,
                       ErrorList *errors);

#endif /* INSTRUCTION_ENCODER_H */

