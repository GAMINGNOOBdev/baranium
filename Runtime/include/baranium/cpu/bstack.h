#ifndef __BARANIUM__BSTACK_H_
#define __BARANIUM__BSTACK_H_

#include <stdint.h>

typedef struct bstackEntry
{
    struct bstackEntry* prev;
    uint64_t data;
    struct bstackEntry* next;
} bstackEntry;

struct bstack;
typedef void(*stack_clear_t)(struct bstack* obj);
typedef void(*stack_push_t)(struct bstack* obj, uint64_t data);
typedef uint64_t(*stack_pop_t)(struct bstack* obj);

typedef struct bstack
{
    bstackEntry* end;
    bstackEntry* start;

    stack_clear_t clear;
    stack_push_t push;
    stack_pop_t pop;
} bstack;

void bstack_init(bstack* obj);

#endif