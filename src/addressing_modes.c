/* addressing_modes.c
 * Helpers to classify operands into registers, immediates, or labels.
 * Used by assembler passes and encoder for validation and parsing.
 */

#include <ctype.h>
#include <string.h>
#include "addressing_modes.h"

/* is_register — true if operand is r0–r7 */
int is_register(const char *operand) {
    return operand && strlen(operand) == 2 &&
           operand[0] == 'r' &&
           operand[1] >= '0' && operand[1] <= '7';
}

/* is_immediate — true if operand starts with '#' followed by an integer */
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

/* is_label — true if operand is a legal label (letter start, then alnum) */
int is_label(const char *operand) {
    int i;
    if (!operand || !isalpha((unsigned char)operand[0])) return 0;
    for (i = 1; operand[i]; i++) {
        if (!isalnum((unsigned char)operand[i])) return 0;
    }
    return 1;
}

