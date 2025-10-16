/* memory_image.h
 * Defines in-memory storage for code, data, and fixups during assembly.
 * Provides functions to init, append words, and record fixups.
 */

#ifndef MEMORY_IMAGE_H
#define MEMORY_IMAGE_H

#include "symbol_table.h"

#define MAX_CODE_SIZE 1024
#define MAX_DATA_SIZE 1024
#define MAX_FIXUPS    1024

/* Fixup — a placeholder for an unresolved symbol reference */
typedef struct {
    int  word_index;                  /* code[] slot to patch */
    char label[MAX_LABEL_LEN];        /* referenced symbol */
    int  line;                        /* source line (for error reporting) */
} Fixup;

/* MemoryImage — holds code, data, and fixups until output stage */
typedef struct {
    int code[MAX_CODE_SIZE];
    int data[MAX_DATA_SIZE];
    int IC;                           /* count of code words */
    int DC;                           /* count of data words */
    Fixup fixups[MAX_FIXUPS];
    int fixup_count;
} MemoryImage;

/* init_memory_image — clear buffers and reset counters */
void init_memory_image(MemoryImage *m);

/* add_code_word — append one code word (10-bit masked) */
void add_code_word(MemoryImage *m, int word);

/* add_data_word — append one data word (10-bit masked) */
void add_data_word(MemoryImage *m, int word);

/* add_fixup — record a symbol reference to patch later in pass-2 */
void add_fixup(MemoryImage *m, int word_index, const char *label, int line);

#endif /* MEMORY_IMAGE_H */

