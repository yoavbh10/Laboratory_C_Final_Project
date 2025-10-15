/* instruction_set.h
 * Defines instruction opcodes, operands, and addressing rules.
 * Provides lookup and debug helpers for instructions.
 */

#ifndef INSTRUCTION_SET_H
#define INSTRUCTION_SET_H

#define MAX_OPCODE_NAME 6   /* Longest name: "stop\0" */
#define NUM_OPCODES 16

/* Addressing method types */
typedef enum {
    ADDR_IMMEDIATE = 0, /* #number */
    ADDR_DIRECT    = 1, /* label */
    ADDR_MATRIX    = 2, /* label[reg][reg] */
    ADDR_REGISTER  = 3  /* r0-r7 */
} AddressingType;

/* Instruction spec: mnemonic, opcode, addressing rules, operand count */
typedef struct {
    char name[MAX_OPCODE_NAME];
    int opcode;
    int allowed_src[4];
    int allowed_dst[4];
    int operands; /* 0, 1, or 2 */
} Instruction;

/* find_instruction — lookup by mnemonic, or NULL if not found */
const Instruction* find_instruction(const char *name);

/* print_instruction_table — debug print of all instructions */
void print_instruction_table(void);

/* get_opcode — return opcode for mnemonic, -1 if unknown */
int get_opcode(const char *mnemonic);

#endif

