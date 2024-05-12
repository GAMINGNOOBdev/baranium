#ifndef __BARANIUM__BCPU_OPCODES_H_
#define __BARANIUM__BCPU_OPCODES_H_ 1

#include <stdint.h>

#define MAX_OPCODE_AMOUNT 0x100

struct bcpu;
typedef void(*OPCODE_HANDLE)(struct bcpu* cpu);

typedef struct bcpu_opcode
{
    const char* name;
    OPCODE_HANDLE handle;
} bcpu_opcode;

void bcpu_opcodes_init();

extern bcpu_opcode opcodes[];

#endif