#include <baranium/cpu/bstack.h>
#include <memory.h>
#include <stdlib.h>

void bstack_init(bstack* obj)
{
    if (obj == NULL)
        return;

    bstack_clear(obj);

    obj->start = NULL;
    obj->end = NULL;
}

void bstack_clear(bstack* obj)
{
    if (obj == NULL)
        return;

    if (obj->start == NULL)
        return;

    bstackEntry* next = NULL;
    for (bstackEntry* ptr = obj->start; ptr != NULL;)
    {
        next = ptr->next;
        free(ptr);
        ptr = next;
    }

    obj->start = obj->end = NULL;
}

void bstack_push(bstack* obj, uint64_t data)
{
    if (obj == NULL)
        return;

    if (obj->start == NULL)
    {
        obj->start = malloc(sizeof(bstackEntry));
        obj->start->prev = NULL;
        obj->start->next = NULL;
        obj->start->data = data;
        obj->end = obj->start;
        return;
    }

    bstackEntry* newEntry = malloc(sizeof(bstackEntry));
    newEntry->prev = obj->end;
    newEntry->data = data;
    newEntry->next = NULL;
    obj->end->next = newEntry;
    obj->end = newEntry;
}

uint64_t bstack_pop(bstack* obj)
{
    if (obj == NULL)
        return 0;

    if (obj->end == NULL)
        return 0;

    bstackEntry* entry = obj->end;
    bstackEntry* prev = obj->end->prev;
    uint64_t data = entry->data;
    free(entry);
    if (prev == NULL)
    {
        obj->start = NULL;
        obj->end = NULL;
        return data;
    }

    prev->next = NULL;
    obj->end = prev;

    return data;
}