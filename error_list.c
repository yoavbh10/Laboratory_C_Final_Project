#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_list.h"

/* Initialize an empty error list */
void init_error_list(ErrorList *list) {
    list->head = NULL;
}

/* Add an error node to the list */
void add_error(ErrorList *list, int line, const char *msg) {
    ErrorNode *new_node = (ErrorNode *)malloc(sizeof(ErrorNode));
    if (!new_node)
        return; /* Out of memory, skip adding */

    new_node->line = line;
    strncpy(new_node->message, msg, ERROR_MSG_LEN - 1);
    new_node->message[ERROR_MSG_LEN - 1] = '\0';
    new_node->next = list->head;
    list->head = new_node;
}

/* Print all errors and clear list */
void print_and_clear_errors(ErrorList *list) {
    ErrorNode *curr = list->head;
    if (!curr) {
        return; /* No errors to print */
    }

    while (curr) {
        ErrorNode *temp = curr; /* C89 safe: declare temp before using */
        printf("Error (line %d): %s\n", curr->line, curr->message);
        curr = curr->next;
        free(temp);
    }
    list->head = NULL;
}

