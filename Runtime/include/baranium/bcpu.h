/**
 * @file bcpu.h
 * @author GAMINGNOOBdev ()
 * @brief Baranium runtime cpu
 * @version 0.1
 * @date 2024-05-09
 * 
 * @note THIS IS NOT INTENDED FOR USE BY THE USER OF THE RUNTIME!
 *       This header is intended to be used internally by the runtime
 *       and therefore, functions defined in this header cannot be used
 *       by the user.
 * 
 * @copyright Copyright (c) GAMINGNOOB 2024
 * 
 */

#ifndef __BARANIUM__BCPU_H_
#define __BARANIUM__BCPU_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "cpu/bstack.h"
#include "cpu/bbus.h"
#include <stdint.h>

typedef struct{
    uint8_t CMP: 1;             // comparisons enable
    uint8_t FORCED_KILL : 1;    // cpu termination has been forced and an error code pushed to the stack
    uint8_t RESERVED: 6;        // reserved
} bcpu_flags;

struct bcpu;
typedef uint64_t(*BCPUFETCH)(struct bcpu* obj, int bits);

typedef struct bcpu
{
    uint64_t IP;            // Instruction Pointer (Program Counter)
    bstack* stack;          // Stack for the cpu to store data temporarily
    bstack* ip_stack;       // Instruction pointer stack for the cpu to store data temporarily
    bcpu_flags flags;       // Flags
    uint8_t opcode;         // operation code/instruction
    uint64_t ticks;         // total number of ticks the cpu has executed
    uint8_t killTriggered;  // Only set if execution has ended
                            // has been triggered and the application should quit
    int64_t cv;             // compare value
    bbus* bus;              // the bus to read data from

    uint64_t fetched;
    BCPUFETCH fetch;
} bcpu;

// create and initialize the cpu
bcpu* bcpu_init();

// dispose and clean any used memory by the cpu
void bcpu_dispose(bcpu* obj);

// this is the method which executes the instructions from the IP value forward
// some instructions can take multiple calls to this function to execute properly
void bcpu_tick(bcpu* obj);

// resets the cpu
void bcpu_reset(bcpu* obj);

#ifdef __cplusplus
}
#endif

#endif