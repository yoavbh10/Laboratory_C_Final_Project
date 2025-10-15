#include <string.h>
#include "memory_image.h"

void init_memory_image(MemoryImage *m) {
    int i;
    if (!m) return;
    for (i = 0; i < MAX_CODE_SIZE; ++i) m->code[i] = 0;
    for (i = 0; i < MAX_DATA_SIZE; ++i) m->data[i] = 0;
    m->IC = 0;
    m->DC = 0;
    m->fixup_count = 0;
}

void add_code_word(MemoryImage *m, int word) {
    if (!m) return;
    if (m->IC < MAX_CODE_SIZE) {
        m->code[m->IC++] = (word & 0x3FF);
    }
}

void add_data_word(MemoryImage *m, int word) {
    if (!m) return;
    if (m->DC < MAX_DATA_SIZE) {
        m->data[m->DC++] = (word & 0x3FF);
    }
}

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

