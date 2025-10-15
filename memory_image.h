#ifndef MEMORY_IMAGE_H
#define MEMORY_IMAGE_H

#include "symbol_table.h" /* for MAX_LABEL_LEN and Symbol type */

#define MAX_CODE_SIZE 256
#define MAX_DATA_SIZE 256
#define MAX_FIXUPS    256

/* A pending relocation to resolve after encoding */
typedef struct {
    int address;                      /* absolute code address (e.g., 100, 101, ...) to patch */
    char label[MAX_LABEL_LEN];        /* target label name ("" if immediate/none) */
    int line;                         /* source line number (for error messages) */
} Fixup;

typedef struct {
    int code[MAX_CODE_SIZE];
    int data[MAX_DATA_SIZE];
    int IC; /* Instruction Counter (logical addresses start at 100) */
    int DC; /* Data Counter */

    /* Second-pass fixups for unresolved labels */
    Fixup fixups[MAX_FIXUPS];
    int fixup_count;
} MemoryImage;

void init_memory_image(MemoryImage *mem);
void add_code_word(MemoryImage *mem, int word);
void add_data_word(MemoryImage *mem, int word);

/* Record a placeholder that must be resolved to a label's address later */
void add_fixup(MemoryImage *mem, int code_address, const char *label, int line);

void print_memory_image(const MemoryImage *mem);

#endif /* MEMORY_IMAGE_H */
