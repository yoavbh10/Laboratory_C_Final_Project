#ifndef ADDRESSING_MODES_H
#define ADDRESSING_MODES_H

#define ADDR_IMMEDIATE 0
#define ADDR_DIRECT    1
#define ADDR_MATRIX    2  /* NEW */
#define ADDR_REGISTER  3

/* Detect if operand is register (r0–r7) */
int is_register(const char *operand);

/* Detect if operand is immediate (#number) */
int is_immediate(const char *operand);

/* Detect if operand is label (valid label name) */
int is_label(const char *operand);

#endif

