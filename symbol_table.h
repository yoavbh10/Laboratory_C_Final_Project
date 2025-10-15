#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#define MAX_LABEL_LEN 32

typedef enum {
    SYMBOL_CODE,
    SYMBOL_DATA
} SymbolType;

typedef struct Symbol {
    char name[MAX_LABEL_LEN];
    int address;
    SymbolType type;
    int is_entry;
    int is_extern;
    struct Symbol *next;
} Symbol;

/* Initialize a symbol table head pointer to NULL */
void init_symbol_table(Symbol **head);

int add_symbol(Symbol **head, const char *name, int address, SymbolType type);
Symbol *find_symbol(Symbol *head, const char *name);
void print_symbol_table(Symbol *head);
void free_symbol_table(Symbol **head);

#endif

