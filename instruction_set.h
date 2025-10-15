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

/* Instruction specification */
typedef struct {
    char name[MAX_OPCODE_NAME];
    int opcode;
    int allowed_src[4]; /* which addressing modes allowed for source */
    int allowed_dst[4]; /* which addressing modes allowed for destination */
    int operands;       /* number of operands: 0, 1, or 2 */
} Instruction;

/* Look up an instruction by name, returns pointer or NULL if not found */
const Instruction* find_instruction(const char *name);

/* Debug: print all instructions */
void print_instruction_table(void);

#endif

