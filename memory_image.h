#ifndef MEMORY_IMAGE_H
#define MEMORY_IMAGE_H

#define MAX_CODE_IMAGE 256
#define MAX_DATA_IMAGE 256

typedef struct
{
    int code[MAX_CODE_IMAGE];
    int data[MAX_DATA_IMAGE];
    int IC;
    int DC;
} MemoryImage;

void init_memory_image(MemoryImage *mem);
void add_code_word(MemoryImage *mem, int word);
void add_data_word(MemoryImage *mem, int word);

#endif

