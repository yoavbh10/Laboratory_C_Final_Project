/* memory_image.c
 * Manages in-memory storage of code, data, and fixups during assembly.
 * Provides init and append functions for words and fixup tracking.
 */

#include <string.h>
#include "memory_image.h"

/* init_memory_image — reset all fields and clear buffers */
void init_memory_image(MemoryImage *m) {
    int i;
    if (!m) return;
    for (i = 0; i < MAX_CODE_SIZE; ++i) m->code[i] = 0;
    for (i = 0; i < MAX_DATA_SIZE; ++i) m->data[i] = 0;
    m->IC = 0;
    m->DC = 0;
    m->fixup_count = 0;
}

/* add_code_word — append one code word to image (10-bit masked) */
void add_code_word(MemoryImage *m, int word) {
    if (!m) return;
    if (m->IC < MAX_CODE_SIZE) {
        m->code[m->IC++] = (word & 0x3FF);
    }
}

/* add_data_word — append one data word to image (10-bit masked) */
void add_data_word(MemoryImage *m, int word) {
    if (!m) return;
    if (m->DC < MAX_DATA_SIZE) {
        m->data[m->DC++] = (word & 0x3FF);
    }
}

/* add_fixup — record a symbol reference to be resolved in pass-2 */
void add_fixup(MemoryImage *m, int word_index, const char *label, int line) {
    Fixup *fx;
    if (!m || !label) return;
    if (m->fixup_count >= MAX_FIXUPS) return;
    fx = &m->fixups[m->fixup_count++];
    fx->word_index = word_index;
    strncpy(fx->label, label, MAX_LABEL_LEN - 1);
    fx->label[MAX_LABEL_LEN - 1] = '\0';
    fx->line = line;
}

