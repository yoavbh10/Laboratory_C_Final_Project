#ifndef ERROR_LIST_H
#define ERROR_LIST_H

#define ERROR_MSG_LEN 100

/* Node representing a single error */
typedef struct ErrorNode {
    int line;
    char message[ERROR_MSG_LEN];
    struct ErrorNode *next;
} ErrorNode;

/* Container for all errors */
typedef struct {
    ErrorNode *head;
} ErrorList;

/* Initialize an error list */
void init_error_list(ErrorList *list);

/* Add an error message for a given line number */
void add_error(ErrorList *list, int line, const char *msg);

/* Print and clear all errors */
void print_and_clear_errors(ErrorList *list);

#endif

