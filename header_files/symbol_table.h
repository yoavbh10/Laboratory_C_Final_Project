/* symbol_table.h
 * Linked-list symbol table for labels, extern, and entry tracking.
 * Supports add, lookup, print, and free operations.
 */

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

/* init_symbol_table — set head pointer to NULL */
void init_symbol_table(Symbol **head);

/* add_symbol — append a new symbol to table */
int add_symbol(Symbol **head, const char *name, int address, SymbolType type);

/* find_symbol — lookup symbol by name, return pointer or NULL */
Symbol *find_symbol(Symbol *head, const char *name);

/* print_symbol_table — debug print of all symbols */
void print_symbol_table(Symbol *head);

/* free_symbol_table — free all nodes and reset head */
void free_symbol_table(Symbol **head);

#endif /* SYMBOL_TABLE_H */

