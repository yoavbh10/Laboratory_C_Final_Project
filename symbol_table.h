#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#define MAX_LABEL_LEN 32

typedef enum {
    SYMBOL_CODE,
    SYMBOL_DATA,
    SYMBOL_EXTERNAL
} SymbolType;

typedef struct Symbol {
    char name[MAX_LABEL_LEN];
    int address;
    SymbolType type;
    struct Symbol *next;
} Symbol;

int add_symbol(Symbol **head, const char *name, int address, SymbolType type);
Symbol *find_symbol(Symbol *head, const char *name);
void print_symbol_table(Symbol *head);
void free_symbol_table(Symbol **head);

#endif

