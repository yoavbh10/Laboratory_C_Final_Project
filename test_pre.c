#include <stdio.h>
#include "pre_assembler.h"
#include "error_list.h"

int main() {
    ErrorList errors;

    init_error_list(&errors);

    if (!pre_assembler("test_macros.as", &errors)) {
        printf("Pre-assembler failed.\n");
        print_and_clear_errors(&errors);
        return 1;
    }

    printf("Pre-assembler completed successfully.\n");
    print_and_clear_errors(&errors);
    return 0;
}

