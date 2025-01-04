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

#include <stdint.h>

typedef struct bstack_entry
{
    struct bstack_entry* prev;
    uint64_t data;
    struct bstack_entry* next;
} bstack_entry;

typedef struct bstack
{
    bstack_entry* end;
    bstack_entry* start;
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
