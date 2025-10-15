#ifndef MEMORY_IMAGE_H
#define MEMORY_IMAGE_H

#define MAX_CODE_SIZE 256
#define MAX_DATA_SIZE 256

typedef struct {
    int code[MAX_CODE_SIZE];
    int data[MAX_DATA_SIZE];
    int IC; /* Instruction Counter */
    int DC; /* Data Counter */
} MemoryImage;

void init_memory_image(MemoryImage *mem);
void add_code_word(MemoryImage *mem, int word);
void add_data_word(MemoryImage *mem, int word);
void print_memory_image(const MemoryImage *mem);

#endif

