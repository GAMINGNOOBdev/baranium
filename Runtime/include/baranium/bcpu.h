#ifndef __BARANIUM__BCPU_H_
#define __BARANIUM__BCPU_H_ 1

#include <stdint.h>

typedef struct{
    uint8_t C: 1;       // carry bit
    uint8_t Z: 1;       // zero bit
    uint8_t I: 1;       // interrupt disable
    uint8_t D: 1;       // decimal mode
    uint8_t B: 1;       // break
    uint8_t OV: 1;      // overlow bit
    uint8_t N: 1;       // negative bit
    uint8_t UNKNOWN: 1; // unknown/reserved
} bcpu_flags;

struct bcpu;
typedef void(*BCPUFETCH)(struct bcpu* _this);

typedef struct bcpu
{
    uint16_t IP;        // Instruction Pointer (Program Counter)
    
    uint16_t stack;      // stack for the cpu to store stuff to

    uint8_t A, X, Y;    // Accumulator, X and Y registers

    bcpu_flags flags;  // Flags

    uint8_t opcode;     // operation code/instruction

    uint8_t ticks;     // ticks needed for the operations to be completed

    #ifdef _DEBUG
    uint64_t totalTicks; // total amount of ticks
    #endif

    uint8_t killTriggered; // Whether the kill instruction has been triggered and the application should quit

    uint16_t ADDR_ABS;  // Absolute address of the next instruction or something
    uint16_t ADDR_REL;  // Relative address added to some absolute value to get the full address

    uint8_t fetched;
    BCPUFETCH fetch;
} bcpu;

// initialize the cpu
void bcpu_init(bcpu* _this);

// this is the method which executes the instructions from the IP value forward
// some instructions can take multiple calls to this function to execute properly
void bcpu_tick(bcpu* _this);

// resets the cpu
void bcpu_reset(bcpu* _this);

// non-maskable interrupt
void bcpu_nmi(bcpu* _this);

// interrupt request
void bcpu_irq(bcpu* _this);

#endif