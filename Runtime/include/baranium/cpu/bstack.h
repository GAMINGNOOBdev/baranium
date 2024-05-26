#ifndef __BARANIUM__BSTACK_H_
#define __BARANIUM__BSTACK_H_

#include <stdint.h>

typedef struct bstackEntry
{
    struct bstackEntry* prev;
    uint64_t data;
    struct bstackEntry* next;
} bstackEntry;

typedef struct bstack
{
    bstackEntry* end;
    bstackEntry* start;
} bstack;

void bstack_init(bstack* obj);
void bstack_clear(bstack* obj);
void bstack_push(bstack* obj, uint64_t data);
uint64_t bstack_pop(bstack* obj);

#endif