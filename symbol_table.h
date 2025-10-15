#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#define MAX_LABEL_LEN 32

/* Symbol types */
typedef enum {
	SYMBOL_CODE,
	SYMBOL_DATA,
	SYMBOL_EXTERNAL
} SymbolType;

/* Symbol node structure */
typedef struct Symbol {
    char name[MAX_LABEL_LEN];
    int address;
    SymbolType type;
    struct Symbol *next;
} Symbol;

/* Add a symbol to the table.
   Returns 0 on success, -1 if symbol already exists. */
void add_symbol(Symbol **head, const char *name, int address, SymbolType type);

/* Find symbol by name.
   Returns pointer to symbol or NULL if not found. */
Symbol *find_symbol(Symbol *head, const char *name);

/* Free all symbols in the table */
void free_symbol_table(Symbol *head);

/* Debug: print all symbols (for testing only) */
void print_symbol_table(Symbol *head);

/* NEW: adjust data symbol addresses after first pass */
void adjust_data_symbol_addresses(Symbol *head, int ic_offset);

#endif


