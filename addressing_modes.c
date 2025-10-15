#include <ctype.h>
#include <string.h>
#include "addressing_modes.h"

/* Check if string is a valid register: r0-r7 */
int is_register(const char *operand) {
    return operand && strlen(operand) == 2 &&
           operand[0] == 'r' &&
           operand[1] >= '0' && operand[1] <= '7';
}

/* Check if string is an immediate: starts with '#' and followed by number */
int is_immediate(const char *operand) {
    int i = 1;
    if (!operand || operand[0] != '#') return 0;
    if (operand[1] == '+' || operand[1] == '-') i++;
    if (!isdigit((unsigned char)operand[i])) return 0;
    for (; operand[i]; i++) {
        if (!isdigit((unsigned char)operand[i])) return 0;
    }
    return 1;
}

/* Check if string is a label: starts with letter, rest alphanumeric */
int is_label(const char *operand) {
    int i;
    if (!operand || !isalpha((unsigned char)operand[0])) return 0;
    for (i = 1; operand[i]; i++) {
        if (!isalnum((unsigned char)operand[i])) return 0;
    }
    return 1;
}

