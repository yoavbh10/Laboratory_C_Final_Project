#include "memory_image.h"

void init_memory_image(MemoryImage *mem)
{
    int i;
    for (i = 0; i < MAX_CODE_IMAGE; i++)
        mem->code[i] = 0;
    for (i = 0; i < MAX_DATA_IMAGE; i++)
        mem->data[i] = 0;
    mem->IC = 100;
    mem->DC = 0;
}

void add_code_word(MemoryImage *mem, int word)
{
    mem->code[mem->IC++ - 100] = word;
}

void add_data_word(MemoryImage *mem, int word)
{
    mem->data[mem->DC++] = word;
}

