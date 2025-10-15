#include "memory_image.h"
#include <stdio.h>

void init_memory_image(MemoryImage *mem)
{
    int i;
    for (i = 0; i < MAX_CODE_SIZE; i++)
        mem->code[i] = 0;
    for (i = 0; i < MAX_DATA_SIZE; i++)
        mem->data[i] = 0;
    mem->IC = 100; /* starting code address */
    mem->DC = 0;
}

void add_code_word(MemoryImage *mem, int word)
{
    if (mem->IC - 100 < MAX_CODE_SIZE)
        mem->code[mem->IC++ - 100] = word;
}

void add_data_word(MemoryImage *mem, int word)
{
    if (mem->DC < MAX_DATA_SIZE)
        mem->data[mem->DC++] = word;
}

void print_memory_image(const MemoryImage *mem)
{
    int i;
    printf("Code Image (IC=%d):\n", mem->IC);
    for (i = 100; i < mem->IC; i++)
        printf("  [%d] %d\n", i, mem->code[i - 100]);

    printf("Data Image (DC=%d):\n", mem->DC);
    for (i = 0; i < mem->DC; i++)
        printf("  [%d] %d\n", i, mem->data[i]);
}

