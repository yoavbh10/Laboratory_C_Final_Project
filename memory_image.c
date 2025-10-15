#include "memory_image.h"
#include <stdio.h>
#include <string.h>

void init_memory_image(MemoryImage *mem)
{
    int i;
    for (i = 0; i < MAX_CODE_SIZE; i++)
        mem->code[i] = 0;
    for (i = 0; i < MAX_DATA_SIZE; i++)
        mem->data[i] = 0;

    mem->IC = 100; /* starting code address */
    mem->DC = 0;

    mem->fixup_count = 0;
    for (i = 0; i < MAX_FIXUPS; i++) {
        mem->fixups[i].address = 0;
        mem->fixups[i].label[0] = '\0';
        mem->fixups[i].line = 0;
    }
}

void add_code_word(MemoryImage *mem, int word)
{
    int idx = mem->IC - 100; /* convert absolute address to code[] index */
    if (idx >= 0 && idx < MAX_CODE_SIZE) {
        mem->code[idx] = word;
        mem->IC++;
    }
}

void add_data_word(MemoryImage *mem, int word)
{
    if (mem->DC < MAX_DATA_SIZE)
        mem->data[mem->DC++] = word;
}

void add_fixup(MemoryImage *mem, int code_address, const char *label, int line)
{
    int idx;
    if (mem->fixup_count >= MAX_FIXUPS)
        return; /* silently drop if out of capacity; caller already logs errors */

    idx = mem->fixup_count++;
    mem->fixups[idx].address = code_address;
    mem->fixups[idx].line = line;

    if (label) {
        /* copy with truncation and ensure NUL */
        strncpy(mem->fixups[idx].label, label, MAX_LABEL_LEN - 1);
        mem->fixups[idx].label[MAX_LABEL_LEN - 1] = '\0';
    } else {
        mem->fixups[idx].label[0] = '\0';
    }
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

