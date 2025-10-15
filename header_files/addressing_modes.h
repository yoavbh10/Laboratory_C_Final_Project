/* addressing_modes.h
 * Detects operand types: register, immediate, or label.
 * Shared by assembler passes and encoder.
 */

#ifndef ADDRESSING_MODES_H
#define ADDRESSING_MODES_H

#define ADDR_IMMEDIATE 0
#define ADDR_DIRECT    1
#define ADDR_MATRIX    2
#define ADDR_REGISTER  3

/* is_register — true if operand is r0–r7 */
int is_register(const char *operand);

/* is_immediate — true if operand is #number */
int is_immediate(const char *operand);

/* is_label — true if operand is a valid label */
int is_label(const char *operand);

#endif

