/* error_list.h
 * Linked list of errors with line numbers.
 * Used by passes to collect and report issues.
 */

#ifndef ERROR_LIST_H
#define ERROR_LIST_H

#define ERROR_MSG_LEN 100

/* One error node (line + message) */
typedef struct ErrorNode {
    int line;
    char message[ERROR_MSG_LEN];
    struct ErrorNode *next;
} ErrorNode;

/* List container (head pointer) */
typedef struct {
    ErrorNode *head;
} ErrorList;

/* init_error_list — set list head to NULL */
void init_error_list(ErrorList *list);

/* add_error — append new error (line + message) */
void add_error(ErrorList *list, int line, const char *msg);

/* print_and_clear_errors — dump errors and free list */
void print_and_clear_errors(ErrorList *list);

#endif

