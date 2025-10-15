#ifndef PRE_ASSEMBLER_H
#define PRE_ASSEMBLER_H

#include <stddef.h>
#include "error_list.h"

/* Expand macros from src_path into an output file with the same base and ".amx" extension.
   - out_path receives the path actually written (e.g., "file.amx")
   - Returns 1 on success, 0 on failure (errors are pushed to ErrorList)
   - The output is always produced even if there are no macros (pass-through)
*/
int pre_assemble(const char *src_path,
                 char *out_path, size_t out_path_sz,
                 ErrorList *errors);

#endif /* PRE_ASSEMBLER_H */

