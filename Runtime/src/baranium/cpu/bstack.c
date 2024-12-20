#include <baranium/cpu/bstack.h>
#include <memory.h>
#include <stdlib.h>

bstack* bstack_init()
{
    bstack* obj = malloc(sizeof(bstack));
    if (obj == NULL) return NULL;

    memset(obj, 0, sizeof(bstack));
    bstack_clear(obj);

    obj->start = NULL;
    obj->end = NULL;

    return obj;
}

void bstack_dispose(bstack* obj)
{
    if (!obj) return;

    bstack_clear(obj);
    free(obj);
}

void bstack_clear(bstack* obj)
{
    if (obj == NULL) return;

    if (obj->start == NULL) return;

    bstack_entry* next = NULL;
    for (bstack_entry* ptr = obj->start; ptr != NULL;)
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
        obj->start = malloc(sizeof(bstack_entry));
        if (!obj->start)
            return;
        memset(obj->start, 0, sizeof(bstack_entry));
        obj->start->prev = NULL;
        obj->start->next = NULL;
        obj->start->data = data;
        obj->end = obj->start;
        return;
    }

    bstack_entry* newEntry = malloc(sizeof(bstack_entry));
    if (!newEntry)
        return;
    memset(newEntry, 0, sizeof(bstack_entry));
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

    bstack_entry* entry = obj->end;
    bstack_entry* prev = obj->end->prev;
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
