#ifndef __BARANIUM__BCPU_OPCODES_H_
#define __BARANIUM__BCPU_OPCODES_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MAX_OPCODE_AMOUNT 0x100

struct bcpu;
typedef void(*OPCODE_HANDLE)(struct bcpu* cpu);

typedef struct
{
    const char* name;
    OPCODE_HANDLE handle;
} bcpu_opcode;

void bcpu_opcodes_init();

extern bcpu_opcode opcodes[];

#ifdef __cplusplus
}
#endif

#endif
