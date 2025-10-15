#ifndef MACRO_TABLE_H
#define MACRO_TABLE_H

#define MAX_MACRO_NAME_LEN 31
#define MAX_MACRO_VALUE_LEN 81

/* Struct representing a macro */
typedef struct Macro {
    char name[MAX_MACRO_NAME_LEN];
    char value[MAX_MACRO_VALUE_LEN];
    struct Macro* next;
} Macro;

/* Adds a macro to the list. Returns 0 on success, -1 if name exists. */
int add_macro(Macro** head, const char* name, const char* value);

/* Finds a macro by name. Returns pointer to value, or NULL if not found. */
const char* find_macro(const Macro* head, const char* name);

/* Frees the entire macro table */
void free_macro_table(Macro* head);

#endif

