#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_list.h"

void init_error_list(ErrorList *list) {
    list->head = NULL;
}

void add_error(ErrorList *list, int line, const char *msg) {
    ErrorNode *new_node = (ErrorNode *)malloc(sizeof(ErrorNode));
    if (!new_node)
        return; /* If malloc fails, silently ignore (rare in small projects) */

    new_node->line = line;
    strncpy(new_node->message, msg, sizeof(new_node->message) - 1);
    new_node->message[sizeof(new_node->message) - 1] = '\0';
    new_node->next = list->head;
    list->head = new_node;
}

int has_errors(const ErrorList *list) {
    return list->head != NULL;
}

void print_and_clear_errors(ErrorList *list) {
    ErrorNode *curr = list->head;
    while (curr) {
        printf("Error (line %d): %s\n", curr->line, curr->message);
        ErrorNode *temp = curr;
        curr = curr->next;
        free(temp);
    }
    list->head = NULL;
}

void free_error_list(ErrorList *list) {
    ErrorNode *curr = list->head;
    while (curr) {
        ErrorNode *temp = curr;
        curr = curr->next;
        free(temp);
    }
    list->head = NULL;
}

