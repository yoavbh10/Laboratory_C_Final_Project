#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

void add_symbol(Symbol **head, const char *name, int address, SymbolType type) {
    Symbol *new_symbol = (Symbol *)malloc(sizeof(Symbol));
    strncpy(new_symbol->name, name, MAX_LABEL_LEN - 1);
    new_symbol->name[MAX_LABEL_LEN - 1] = '\0';
    new_symbol->address = address;
    new_symbol->type = type;
    new_symbol->next = *head;
    *head = new_symbol;
}

Symbol *find_symbol(Symbol *head, const char *name) {
    while (head) {
        if (strcmp(head->name, name) == 0)
            return head;
        head = head->next;
    }
    return NULL;
}

void free_symbol_table(Symbol *head) {
    while (head) {
        Symbol *temp = head;
        head = head->next;
        free(temp);
    }
}

void print_symbol_table(Symbol *head) {
    while (head) {
        const char *type_str = (head->type == SYMBOL_CODE) ? "CODE" :
                               (head->type == SYMBOL_DATA) ? "DATA" : "EXTERNAL";
        printf("Symbol: %-32s Address: %4d  Type: %s\n",
               head->name, head->address, type_str);
        head = head->next;
    }
}

/* NEW: adjust all data symbols to be offset by final IC */
void adjust_data_symbol_addresses(Symbol *head, int ic_offset) {
    while (head) {
        if (head->type == SYMBOL_DATA) {
            head->address += ic_offset;
        }
        head = head->next;
    }
}

