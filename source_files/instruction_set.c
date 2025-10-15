/* instruction_set.c
 * Defines the instruction table (opcodes, operands, addressing rules).
 * Provides lookup and debug helpers for instructions.
 */

#include <stdio.h>
#include <string.h>
#include "instruction_set.h"
#include "addressing_modes.h"

/*
  Addressing-mode index order (must match all other modules):
    0 = ADDR_IMMEDIATE
    1 = ADDR_DIRECT
    2 = ADDR_MATRIX
    3 = ADDR_REGISTER
*/

/* name, opcode, allowed_src[4], allowed_dst[4], operands */
static Instruction instructions[NUM_OPCODES] = {
    /*           imm dir mat reg    imm dir mat reg   ops */
    {"mov",   0, { 1,  1,  1,  1 }, { 0,  1,  1,  1 }, 2},
    {"cmp",   1, { 1,  1,  1,  1 }, { 1,  1,  1,  1 }, 2},
    {"add",   2, { 1,  1,  1,  1 }, { 0,  1,  1,  1 }, 2},
    {"sub",   3, { 1,  1,  1,  1 }, { 0,  1,  1,  1 }, 2},
    {"not",   4, { 0,  0,  0,  0 }, { 0,  1,  1,  1 }, 1},
    {"clr",   5, { 0,  0,  0,  0 }, { 0,  1,  1,  1 }, 1},
    {"lea",   6, { 0,  1,  1,  0 }, { 0,  1,  1,  1 }, 2},
    {"inc",   7, { 0,  0,  0,  0 }, { 0,  1,  1,  1 }, 1},
    {"dec",   8, { 0,  0,  0,  0 }, { 0,  1,  1,  1 }, 1},
    {"jmp",   9, { 0,  0,  0,  0 }, { 0,  1,  1,  1 }, 1},
    {"bne",  10, { 0,  0,  0,  0 }, { 0,  1,  1,  1 }, 1},
    {"red",  11, { 0,  0,  0,  0 }, { 0,  1,  1,  1 }, 1},
    {"prn",  12, { 0,  0,  0,  0 }, { 1,  1,  1,  1 }, 1},
    {"jsr",  13, { 0,  0,  0,  0 }, { 0,  1,  1,  1 }, 1},
    {"rts",  14, { 0,  0,  0,  0 }, { 0,  0,  0,  0 }, 0},
    {"stop", 15, { 0,  0,  0,  0 }, { 0,  0,  0,  0 }, 0}
};

/* find_instruction — lookup instruction by name */
const Instruction* find_instruction(const char *name) {
    int i;
    for (i = 0; i < NUM_OPCODES; i++) {
        if (strcmp(instructions[i].name, name) == 0)
            return &instructions[i];
    }
    return NULL;
}

/* print_instruction_table — debug print of instruction set */
void print_instruction_table(void) {
    int i;
    for (i = 0; i < NUM_OPCODES; i++) {
        printf("%-5s opcode=%2d operands=%d\n",
               instructions[i].name, instructions[i].opcode, instructions[i].operands);
    }
}

/* get_opcode — return opcode for mnemonic, or -1 if unknown */
int get_opcode(const char *mnemonic) {
    int i;
    for (i = 0; i < NUM_OPCODES; i++) {
        if (strcmp(instructions[i].name, mnemonic) == 0)
            return instructions[i].opcode;
    }
    return -1; /* unknown mnemonic */
}

