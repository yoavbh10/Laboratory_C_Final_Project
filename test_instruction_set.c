#include "instruction_set.h"
#include <stdio.h>

int main(void) {
    const Instruction *ins;

    print_instruction_table();

    ins = find_instruction("mov");
    if (ins)
        printf("Found %s, opcode=%d, operands=%d\n", ins->name, ins->opcode, ins->operands);

    ins = find_instruction("xyz");
    if (!ins)
        printf("Instruction xyz not found (OK)\n");

    return 0;
}

