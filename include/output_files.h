/* output_files.h
 * Handles writing .ob/.ent/.ext files and tracking extern symbol uses.
 * Uses base-4 encoding for output words and addresses.
 */

#ifndef OUTPUT_FILES_H
#define OUTPUT_FILES_H

#include "memory_image.h"
#include "symbol_table.h"

/* of_init — reset extern-use tracking for a new file */
void of_init(void);

/* of_record_extern_use — record extern symbol use at absolute address */
void of_record_extern_use(const char *name, int use_address);

/* write_output_files — emit .ob/.ent/.ext files for assembled source */
void write_output_files(const char *src_filename,
                        const MemoryImage *mem,
                        const Symbol *symbols);

#endif /* OUTPUT_FILES_H */

