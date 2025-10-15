#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

int add_symbol(Symbol **head, const char *name, int address, SymbolType type) {
    Symbol *new_symbol = (Symbol *)malloc(sizeof(Symbol));
    Symbol *temp;

    if (!new_symbol) return 0;

    strncpy(new_symbol->name, name, MAX_LABEL_LEN - 1);
    new_symbol->name[MAX_LABEL_LEN - 1] = '\0';
    new_symbol->address = address;
    new_symbol->type = type;
    new_symbol->next = NULL;

    if (!(*head)) {
        *head = new_symbol;
    } else {
        temp = *head;
        while (temp->next) temp = temp->next;
        temp->next = new_symbol;
    }
    return 1;
}

Symbol *find_symbol(Symbol *head, const char *name) {
    while (head) {
        if (strcmp(head->name, name) == 0)
            return head;
        head = head->next;
    }
    return NULL;
}

void print_symbol_table(Symbol *head) {
    while (head) {
        const char *type_str = 
            head->type == SYMBOL_CODE ? "CODE" :
            (head->type == SYMBOL_DATA ? "DATA" : "EXTERNAL");
        printf("Symbol: %-32s Address: %4d  Type: %s\n",
               head->name, head->address, type_str);
        head = head->next;
    }
}

void free_symbol_table(Symbol **head) {
    Symbol *curr = *head;
    Symbol *temp;
    while (curr) {
        temp = curr;
        curr = curr->next;
        free(temp);
    }
    *head = NULL;
}

