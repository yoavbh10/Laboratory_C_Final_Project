#ifndef OUTPUT_FILES_H
#define OUTPUT_FILES_H

#include "memory_image.h"
#include "symbol_table.h"

/* Reset/clear any per-file extern-use tracking. Call before assembling a file. */
void of_init(void);

/* Record a single external symbol use at an absolute code address. */
void of_record_extern_use(const char *name, int use_address);

/* Write .ob / .ent / .ext for the assembled file.
   src_filename is the original source; the function derives the base name.
   This function does not return a value; it assumes inputs are valid and
   emits files conditionally (.ent/.ext only when needed). */
void write_output_files(const char *src_filename,
                        const MemoryImage *mem,
                        const Symbol *symbols);

#endif /* OUTPUT_FILES_H */

