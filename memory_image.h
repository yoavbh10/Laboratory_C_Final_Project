#ifndef MEMORY_IMAGE_H
#define MEMORY_IMAGE_H

#include "symbol_table.h"

#define MAX_CODE_SIZE 1024
#define MAX_DATA_SIZE 1024
#define MAX_FIXUPS    1024

/* Fixup points at a code word index (0-based into code[]).
   During pass-2 we resolve and patch code[word_index]. */
typedef struct {
    int  word_index;                  /* which code[] slot to patch */
    char label[MAX_LABEL_LEN];        /* referenced symbol */
    int  line;                        /* source line for error reporting */
} Fixup;

typedef struct {
    int code[MAX_CODE_SIZE];
    int data[MAX_DATA_SIZE];
    int IC;                           /* count of code words written (0-based) */
    int DC;                           /* count of data words written */
    Fixup fixups[MAX_FIXUPS];
    int fixup_count;
} MemoryImage;

void init_memory_image(MemoryImage *m);
void add_code_word(MemoryImage *m, int word);
void add_data_word(MemoryImage *m, int word);

/* Record a fixup to be resolved in pass-2.
   word_index is the code[] index that needs patching (0-based). */
void add_fixup(MemoryImage *m, int word_index, const char *label, int line);

#endif

