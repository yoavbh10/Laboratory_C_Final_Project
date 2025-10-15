#ifndef ERROR_LIST_H
#define ERROR_LIST_H

/* A single error node */
typedef struct ErrorNode {
    int line;
    char message[100];
    struct ErrorNode *next;
} ErrorNode;

/* Head of linked list */
typedef struct {
    ErrorNode *head;
} ErrorList;

/* Initialize an empty error list */
void init_error_list(ErrorList *list);

/* Add an error (line number + message) */
void add_error(ErrorList *list, int line, const char *msg);

/* Check if there are any errors (1 if yes, 0 if none) */
int has_errors(const ErrorList *list);

/* Print all errors and clear the list */
void print_and_clear_errors(ErrorList *list);

/* Free all error nodes without printing */
void free_error_list(ErrorList *list);

#endif

