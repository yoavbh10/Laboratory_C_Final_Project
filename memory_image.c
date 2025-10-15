#include <stdio.h>
#include "memory_image.h"

void init_memory_image(MemoryImage *mem) {
    int i;
    mem->IC = 100;
    mem->DC = 0;
    for (i = 0; i < MAX_CODE_SIZE; i++)
        mem->code_image[i] = 0;
    for (i = 0; i < MAX_DATA_SIZE; i++)
        mem->data_image[i] = 0;
}

int add_code_word(MemoryImage *mem, int word) {
    int index = mem->IC - 100; /* code starts at address 100 */
    if (index >= MAX_CODE_SIZE)
        return -1;
    mem->code_image[index] = word;
    mem->IC++;
    return 0;
}

int add_data_word(MemoryImage *mem, int word) {
    if (mem->DC >= MAX_DATA_SIZE)
        return -1;
    mem->data_image[mem->DC++] = word;
    return 0;
}

int get_code_word(const MemoryImage *mem, int index) {
    if (index < 0 || index >= (mem->IC - 100))
        return 0;
    return mem->code_image[index];
}

int get_data_word(const MemoryImage *mem, int index) {
    if (index < 0 || index >= mem->DC)
        return 0;
    return mem->data_image[index];
}

void print_memory_image(const MemoryImage *mem) {
    int i;
    printf("Code Image (IC=%d):\n", mem->IC);
    for (i = 0; i < (mem->IC - 100); i++)
        printf("  [%3d] %d\n", 100 + i, mem->code_image[i]);

    printf("Data Image (DC=%d):\n", mem->DC);
    for (i = 0; i < mem->DC; i++)
        printf("  [%3d] %d\n", i, mem->data_image[i]);
}

