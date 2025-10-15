#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#define MAX_LABEL_LENGTH 31

/* Symbol types */
typedef enum {
    SYMBOL_CODE,
    SYMBOL_DATA,
    SYMBOL_EXTERNAL,
    SYMBOL_ENTRY
} SymbolType;

/* Symbol node structure */
typedef struct Symbol {
    char name[MAX_LABEL_LENGTH + 1];
    int address;
    SymbolType type;
    struct Symbol *next;
} Symbol;

/* Add a symbol to the table.
   Returns 0 on success, -1 if symbol already exists. */
int add_symbol(Symbol **head, const char *name, int address, SymbolType type);

/* Find symbol by name.
   Returns pointer to symbol or NULL if not found. */
Symbol* find_symbol(Symbol *head, const char *name);

/* Update all DATA symbols by adding offset (for ICF after first pass). */
void update_data_symbols(Symbol *head, int offset);

/* Free all symbols in the table */
void free_symbol_table(Symbol *head);

/* Debug: print all symbols (for testing only) */
void print_symbol_table(Symbol *head);

#endif

