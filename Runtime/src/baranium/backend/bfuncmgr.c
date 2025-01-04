#include <baranium/backend/bfuncmgr.h>
#include <baranium/logging.h>
#include <memory.h>
#include <stdlib.h>

void bfuncmgr_n_free(bfuncmgr_n* obj)
{
    if (!obj)
        return;

    free(obj);
}

baranium_function_manager* baranium_function_manager_init(void)
{
    baranium_function_manager* obj = malloc(sizeof(baranium_function_manager));
    if (obj == NULL) return NULL;

    memset(obj, 0, sizeof(baranium_function_manager));
    baranium_function_manager_clear(obj);

    obj->start = NULL;
    obj->end = NULL;
    obj->count = 0;

    LOGDEBUG("Created function manager");

    return obj;
}

void baranium_function_manager_dispose(baranium_function_manager* obj)
{
    if (!obj) return;

    LOGDEBUG(stringf("Disposing function manager with %ld entries", obj->count));

    baranium_function_manager_clear(obj);
    free(obj);
}

void baranium_function_manager_clear(baranium_function_manager* obj)
{
    if (obj == NULL) return;

    if (obj->start == NULL) return;

    LOGDEBUG(stringf("Cleared function manager with %ld entries", obj->count));

    bfuncmgr_n* next = NULL;
    for (bfuncmgr_n* ptr = obj->start; ptr != NULL;)
    {
        next = ptr->next;
        bfuncmgr_n_free(ptr);
        ptr = next;
    }

    obj->start = obj->end = NULL;
    obj->count = 0;
}

int baranium_function_manager_add_entry(baranium_function_manager* obj, index_t id, baranium_script* script);

void baranium_function_manager_add(baranium_function_manager* obj, index_t id, baranium_script* script)
{
    if (obj == NULL)
        return;

    if (id == INVALID_INDEX)
        return;

    bfuncmgr_n* func = baranium_function_manager_get_entry(obj, id);
    if (func != NULL)
        return;

    int status = baranium_function_manager_add_entry(obj, id, script);
    if (status)
    {
        LOGERROR(stringf("Could not allocate entry for function with id %ld, status code 0x%2.2x", id, status));
        return;
    }

    LOGDEBUG(stringf("Allocated entry for function with id %ld", id));
}

baranium_function* baranium_function_manager_get(baranium_function_manager* obj, index_t id)
{
    if (!obj)
        return NULL;

    bfuncmgr_n* entry = baranium_function_manager_get_entry(obj, id);
    if (entry == NULL)
        return NULL;

    return baranium_script_get_function_by_id(entry->script, entry->id);
}

bfuncmgr_n* baranium_function_manager_get_entry(baranium_function_manager* obj, index_t id)
{
    if (!obj)
        return NULL;

    if (!obj->start)
        return NULL;

    bfuncmgr_n* entry = obj->start;
    for (; entry != NULL; entry = entry->next)
    {
        if (entry->id == id)
            break;
    }

    if (!entry)
        return NULL;

    return entry;
}

void baranium_function_manager_remove(baranium_function_manager* obj, index_t id)
{
    if (obj == NULL)
        return;

    if (obj->start == NULL)
        return;

    bfuncmgr_n* foundEntry = baranium_function_manager_get_entry(obj, id);
    if (foundEntry == NULL)
    {
        LOGERROR(stringf("Could not find function with id %ld", id));
        return;
    }

    bfuncmgr_n* prev = foundEntry->prev;
    bfuncmgr_n* next = foundEntry->next;

    if (foundEntry == obj->start && foundEntry == obj->end)
    {
        obj->start = obj->end = NULL;
        goto destroy;
    }
    else if (foundEntry == obj->start)
    {
        next->prev = NULL;
        obj->start = next;
        foundEntry->next = NULL;
        goto destroy;
    }
    else if (foundEntry == obj->end)
    {
        prev->next = NULL;
        obj->end = prev;
        foundEntry->prev = NULL;
        goto destroy;
    }

    if (next)
        next->prev = prev;

    if (prev)
        prev->next = next;

    foundEntry->next = NULL;
    foundEntry->prev = NULL;

destroy:
    obj->count--;
    LOGDEBUG(stringf("Disposed function with id %ld", id));
    bfuncmgr_n_free(foundEntry);
}

int baranium_function_manager_add_entry(baranium_function_manager* obj, index_t id, baranium_script* script)
{
    if (obj == NULL || id == INVALID_INDEX || script == NULL)
        return 1;

    if (obj->start == NULL)
    {
        obj->start = malloc(sizeof(bfuncmgr_n));
        if (!obj->start)
            return 2;

        memset(obj->start, 0, sizeof(bfuncmgr_n));
        obj->start->prev = NULL;
        obj->start->next = NULL;
        obj->start->id = id;
        obj->start->script = script;
        obj->end = obj->start;
        obj->count = 1;
        return 0;
    }

    bfuncmgr_n* newEntry = malloc(sizeof(bfuncmgr_n));
    if (!newEntry)
        return 2;

    memset(newEntry, 0, sizeof(bfuncmgr_n));
    newEntry->prev = obj->end;
    newEntry->id = id;
    newEntry->script = script;
    newEntry->next = NULL;

    obj->end->next = newEntry;

    obj->end = newEntry;
    obj->count++;

    return 0;
}
