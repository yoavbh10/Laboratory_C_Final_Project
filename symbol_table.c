#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

int add_symbol(Symbol **head, const char *name, int address, SymbolType type) {
    Symbol *curr = *head;

    /* Check for duplicate name */
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            return -1; /* already exists */
        }
        curr = curr->next;
    }

    /* Allocate new node */
    curr = (Symbol*)malloc(sizeof(Symbol));
    if (!curr) return -1;

    strncpy(curr->name, name, MAX_LABEL_LENGTH);
    curr->name[MAX_LABEL_LENGTH] = '\0'; /* ensure null termination */
    curr->address = address;
    curr->type = type;
    curr->next = *head;

    *head = curr;
    return 0;
}

Symbol* find_symbol(Symbol *head, const char *name) {
    while (head) {
        if (strcmp(head->name, name) == 0) return head;
        head = head->next;
    }
    return NULL;
}

void update_data_symbols(Symbol *head, int offset) {
    while (head) {
        if (head->type == SYMBOL_DATA) {
            head->address += offset;
        }
        head = head->next;
    }
}

void free_symbol_table(Symbol *head) {
    Symbol *temp;
    while (head) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

void print_symbol_table(Symbol *head) {
    const char *type_names[] = {"CODE", "DATA", "EXTERNAL", "ENTRY"};
    while (head) {
        printf("Symbol: %-32s Address: %4d  Type: %s\n",
               head->name, head->address, type_names[head->type]);
        head = head->next;
    }
}

