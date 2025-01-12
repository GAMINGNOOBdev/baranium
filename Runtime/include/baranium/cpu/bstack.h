/**
 * @note THIS IS NOT INTENDED FOR USE BY THE USER OF THE RUNTIME!
 *       This header is intended to be used internally by the runtime
 *       and therefore, functions defined in this header cannot be used
 *       by the user.
 */
#ifndef __BARANIUM__BSTACK_H_
#define __BARANIUM__BSTACK_H_

#ifdef __cplusplus
extern "C" {
#endif

#define INITIAL_STACK_SIZE 0x8000

#include <stdint.h>
#include <stdlib.h>

typedef struct bstack
{
    uint64_t* stackptr;
    size_t bufferSize;
    size_t count;
} bstack;

// create and initialize a stack
bstack* bstack_init(void);

// dispose a stack
void bstack_dispose(bstack* obj);

// clear a stack
void bstack_clear(bstack* obj);

// push a value to the stack
void bstack_push(bstack* obj, uint64_t data);

// pop a value from the stack
uint64_t bstack_pop(bstack* obj);

#ifdef __cplusplus
}
#endif

#endif
