#include <baranium/backend/bvarmgr.h>
#include <baranium/logging.h>
#include <memory.h>
#include <stdlib.h>

bvarmgr* bvarmgr_init()
{
    bvarmgr* obj = malloc(sizeof(bvarmgr));
    if (obj == NULL) return;

    memset(obj, 0, sizeof(bvarmgr));
    bvarmgr_clear(obj);

    obj->start = NULL;
    obj->end = NULL;
    obj->count = 0;

    LOGDEBUG("Created variable manager");

    return obj;
}

void bvarmgr_dispose(bvarmgr* obj)
{
    if (!obj) return;

    LOGDEBUG(stringf("Disposing variable manager with %ld entries", obj->count));

    bvarmgr_clear(obj);
    free(obj);
}

void bvarmgr_clear(bvarmgr* obj)
{
    if (obj == NULL) return;

    if (obj->start == NULL) return;

    LOGDEBUG(stringf("Cleared variable manager with %ld entries", obj->count));

    bvarmgr_n* next = NULL;
    for (bvarmgr_n* ptr = obj->start; ptr != NULL;)
    {
        next = ptr->next;
        if (ptr->variable)
            baranium_variable_dispose(ptr->variable);
        free(ptr);
        ptr = next;
    }

    obj->start = obj->end = NULL;
    obj->count = 0;
}

void bvarmgr_alloc(bvarmgr* obj, enum BaraniumVariableType type, index_t id, size_t size)
{
    if (obj == NULL)
        return;

    if (type == BaraniumVariableType_Invalid || type == BaraniumVariableType_Void)
        return;

    BaraniumVariable* variable = malloc(sizeof(BaraniumVariable));
    if (variable == NULL)
        return;

    memset(variable, 0, sizeof(BaraniumVariable));
    variable->ID = id;
    variable->Type = type;
    variable->Size = size;

    variable->Value = malloc(size); // quick sidenote: even if the object is a string, the nullchar is automatically added to the size in the compilation process
    if (!variable->Value)
    {
        free(variable);
        return;
    }
    memset(variable->Value, 0, size);

    LOGDEBUG(stringf("Allocated variable with id %ld and size %ld", id, size));

    if (obj->start == NULL)
    {
        obj->start = malloc(sizeof(bvarmgr_n));
        if (!obj->start)
        {
            baranium_variable_dispose(variable);
            return;
        }
        memset(obj->start, 0, sizeof(bvarmgr_n));
        obj->start->prev = NULL;
        obj->start->next = NULL;
        obj->start->variable = variable;
        obj->end = obj->start;
        obj->count = 1;
        return;
    }

    bvarmgr_n* newEntry = malloc(sizeof(bvarmgr_n));
    if (!newEntry)
    {
        baranium_variable_dispose(variable);
        return;
    }
    memset(newEntry, 0, sizeof(bvarmgr_n));
    newEntry->prev = obj->end;
    newEntry->variable = variable;
    newEntry->next = NULL;
    obj->end->next = newEntry;
    obj->end = newEntry;
    obj->count++;

    return;
}

BaraniumVariable* bvarmgr_get(bvarmgr* obj, index_t id)
{
    if (!obj)
        return NULL;
    
    if (!obj->start)
        return NULL;

    bvarmgr_n* entry = obj->start;
    for (; entry != NULL; entry = entry->next)
    {
        if (entry->variable == NULL)
            continue;
        
        if (entry->variable->ID != id)
            continue;

        break;
    }

    return entry->variable;
}

void bvarmgr_dealloc(bvarmgr* obj, index_t id)
{
    if (obj == NULL)
        return;

    if (obj->start == NULL)
        return;
    
    bvarmgr_n* foundEntry = NULL;

    for (bvarmgr_n* entry = obj->start; entry != NULL; entry = entry->next)
    {
        if (entry->variable == NULL)
            continue;
        
        if (entry->variable->ID != id)
            continue;

        foundEntry = entry;
        break;
    }

    if (!foundEntry)
        return;

    bvarmgr_n* prev = foundEntry->prev;
    bvarmgr_n* next = foundEntry->next;

    if (foundEntry == obj->start && foundEntry == obj->end)
    {
        obj->count = 0;
        obj->start = obj->end = NULL;
        goto destroy;
    }
    else if (foundEntry == obj->start)
    {
        next->prev = NULL;
        obj->start = next;
        foundEntry->next = NULL;
        obj->count--;
        goto destroy;
    }
    else if (foundEntry == obj->end)
    {
        prev->next = NULL;
        obj->end = prev;
        foundEntry->prev = NULL;
        obj->count--;
        goto destroy;
    }

destroy:
    LOGDEBUG(stringf("Disposed variable with id %ld", id));
    baranium_variable_dispose(foundEntry->variable);
    free(foundEntry);
}