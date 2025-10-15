#ifndef MEMORY_IMAGE_H
#define MEMORY_IMAGE_H

#define MAX_CODE_SIZE 1000
#define MAX_DATA_SIZE 1000

/* Structure representing memory images */
typedef struct {
    int code_image[MAX_CODE_SIZE];
    int data_image[MAX_DATA_SIZE];
    int IC; /* Instruction Counter (starts at 100) */
    int DC; /* Data Counter (starts at 0) */
} MemoryImage;

/* Initialize memory image (set counters and clear arrays) */
void init_memory_image(MemoryImage *mem);

/* Add a word to code image, returns 0 on success, -1 if overflow */
int add_code_word(MemoryImage *mem, int word);

/* Add a word to data image, returns 0 on success, -1 if overflow */
int add_data_word(MemoryImage *mem, int word);

/* Get a code word by index (0-based from IC start) */
int get_code_word(const MemoryImage *mem, int index);

/* Get a data word by index (0-based) */
int get_data_word(const MemoryImage *mem, int index);

/* Debug print memory image */
void print_memory_image(const MemoryImage *mem);

#endif

