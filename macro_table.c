#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "macro_table.h"

int add_macro(Macro** head, const char* name, const char* value) {
    Macro* curr = *head;

    /* Check for duplicate */
    while (curr != NULL) {
        if (strcmp(curr->name, name) == 0)
            return -1;
        curr = curr->next;
    }

    /* Allocate new macro */
    curr = (Macro*)malloc(sizeof(Macro));
    if (curr == NULL)
        return -1;

    strncpy(curr->name, name, MAX_MACRO_NAME_LEN - 1);
    curr->name[MAX_MACRO_NAME_LEN - 1] = '\0';
    strncpy(curr->value, value, MAX_MACRO_VALUE_LEN - 1);
    curr->value[MAX_MACRO_VALUE_LEN - 1] = '\0';

    curr->next = *head;
    *head = curr;
    return 0;
}

const char* find_macro(const Macro* head, const char* name) {
    while (head != NULL) {
        if (strcmp(head->name, name) == 0)
            return head->value;
        head = head->next;
    }
    return NULL;
}

void free_macro_table(Macro* head) {
    Macro* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

