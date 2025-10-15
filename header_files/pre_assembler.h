/* pre_assembler.h
 * Expands macros in .as files into .am files.
 * Public API for the pre-assembler stage.
 */
 
#ifndef PRE_ASSEMBLER_H
#define PRE_ASSEMBLER_H

#include <stddef.h>
#include "error_list.h"

/* pre_assemble — expands macros in src (.as) into .am file */
int pre_assemble(const char *src_path,
                 char *out_path, size_t out_path_sz,
                 ErrorList *errors);

#endif /* PRE_ASSEMBLER_H */

