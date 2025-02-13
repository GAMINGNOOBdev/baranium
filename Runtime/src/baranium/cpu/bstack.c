#include <baranium/cpu/bstack.h>
#include <memory.h>
#include <stdlib.h>

bstack* bstack_init(void)
{
    bstack* obj = malloc(sizeof(bstack));
    if (obj == NULL) return NULL;

    memset(obj, 0, sizeof(bstack));
    obj->stackptr = malloc(sizeof(uint64_t)*INITIAL_STACK_SIZE);
    if (obj->stackptr == NULL)
    {
        free(obj);
        return NULL;
    }
    memset(obj->stackptr, 0, INITIAL_STACK_SIZE);
    obj->buffer_size = INITIAL_STACK_SIZE;
    obj->count = 0;

    return obj;
}

void bstack_dispose(bstack* obj)
{
    if (!obj) return;

    free(obj->stackptr);
    free(obj);
}

void bstack_clear(bstack* obj)
{
    if (obj == NULL) return;
    if (obj->stackptr == NULL) return;
    memset(obj->stackptr, 0, obj->buffer_size);
    obj->count = 0;
}

void bstack_push(bstack* obj, uint64_t data)
{
    if (obj == NULL)
        return;

    if (obj->count * sizeof(uint64_t) == obj->buffer_size)
    {
        obj->buffer_size += INITIAL_STACK_SIZE;
        obj->stackptr = realloc(obj->stackptr, obj->buffer_size);
    }
    obj->stackptr[obj->count] = data;
    obj->count++;
}

uint64_t bstack_pop(bstack* obj)
{
    if (obj == NULL)
        return 0;

    if (obj->count == 0)
        return 0;

    obj->count--;
    return obj->stackptr[obj->count];
}
