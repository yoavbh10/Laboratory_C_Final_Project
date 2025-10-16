/* symbol_table.c
 * Manages linked-list of symbols (labels, extern, entry).
 * Supports add, lookup, print, and free.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

/* init_symbol_table — initialize table to empty */
void init_symbol_table(Symbol **head) {
    if (head) *head = NULL;
}

/* add_symbol — append a new symbol to the table */
int add_symbol(Symbol **head, const char *name, int address, SymbolType type)
{
    Symbol *new_symbol = (Symbol *)malloc(sizeof(Symbol));
    Symbol *current;

    if (!new_symbol)
        return 0;

    strcpy(new_symbol->name, name);
    new_symbol->address = address;
    new_symbol->type = type;
    new_symbol->is_entry = 0;
    new_symbol->is_extern = 0;
    new_symbol->next = NULL;

    if (*head == NULL) {
        *head = new_symbol;
        return 1;
    }

    current = *head;
    while (current->next)
        current = current->next;
    current->next = new_symbol;

    return 1;
}

/* find_symbol — return pointer to symbol by name, or NULL */
Symbol *find_symbol(Symbol *head, const char *name)
{
    while (head) {
        if (strcmp(head->name, name) == 0)
            return head;
        head = head->next;
    }
    return NULL;
}

/* print_symbol_table — debug print of all symbols */
void print_symbol_table(Symbol *head)
{
    while (head) {
        printf("Symbol: %-32s Address:%6d  Type:%-6s Entry:%d Extern:%d\n",
               head->name, head->address,
               head->type == SYMBOL_CODE ? "CODE" : "DATA",
               head->is_entry, head->is_extern);
        head = head->next;
    }
}

/* free_symbol_table — free all nodes and reset head */
void free_symbol_table(Symbol **head)
{
    Symbol *curr = *head;
    while (curr) {
        Symbol *temp = curr;
        curr = curr->next;
        free(temp);
    }
    *head = NULL;
}

