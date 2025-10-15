#ifndef OUTPUT_FILES_H
#define OUTPUT_FILES_H

#include "memory_image.h"
#include "symbol_table.h"

/* Reset recorded extern uses (call before assembling a new file). */
void of_reset(void);

/* Record one extern use at logical code address 'addr'. */
void of_record_extern_use(const char *name, int addr);

/* Write .ob / .ent / .ext using a base derived from src_filename (strips .am/.amx). */
void write_output_files(const char *src_filename,
                        const MemoryImage *mem,
                        const Symbol *symbols);

#endif /* OUTPUT_FILES_H */

