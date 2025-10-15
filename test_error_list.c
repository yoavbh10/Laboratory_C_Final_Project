#include "error_list.h"
#include <stdio.h>

int main(void) {
    ErrorList errors;
    init_error_list(&errors);

    add_error(&errors, 1, "Unexpected token");
    add_error(&errors, 2, "Label redefined");
    add_error(&errors, 10, "Unknown instruction");

    if (has_errors(&errors))
        printf("Errors found:\n");

    print_and_clear_errors(&errors);

    if (!has_errors(&errors))
        printf("All errors cleared successfully.\n");

    return 0;
}

