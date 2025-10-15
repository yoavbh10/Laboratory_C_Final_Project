#include "memory_image.h"
#include <stdio.h>

int main(void) {
    MemoryImage mem;
    init_memory_image(&mem);

    add_code_word(&mem, 512);
    add_code_word(&mem, 1023);
    add_data_word(&mem, 123);
    add_data_word(&mem, -45);

    printf("Testing Memory Image:\n");
    print_memory_image(&mem);

    printf("Code word 0: %d\n", get_code_word(&mem, 0));
    printf("Data word 1: %d\n", get_data_word(&mem, 1));

    return 0;
}

