#include <baranium/runtime.h>
#include <baranium/callback.h>
#include <baranium/logging.h>
#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_APPLE
#   include <stdlib.h>
#else
#   include <malloc.h>
#endif
#include <memory.h>

internal_operation_t instantiate_callback = NULL;
internal_operation_t delete_callback = NULL;
internal_operation_t attach_callback = NULL;
internal_operation_t detach_callback = NULL;

void baranium_callback_set_internal_operations(internal_operation_t instantiateCB, internal_operation_t deleteCB, internal_operation_t attachCB, internal_operation_t detachCB)
{
    instantiate_callback = instantiateCB;
    delete_callback = deleteCB;
    attach_callback = attachCB;
    detach_callback = detachCB;
}

baranium_callback_list* baranium_callback_list_init(void)
{
    baranium_callback_list* list = malloc(sizeof(baranium_callback_list));
    if (list == NULL) return NULL;

    memset(list, 0, sizeof(baranium_callback_list));

    list->start = NULL;
    list->end = NULL;
    list->count = 0;

    LOGDEBUG("Created callback list");

    return list;
}

void baranium_callback_list_dispose(baranium_callback_list* list)
{
    if (!list) return;

    LOGDEBUG(stringf("Disposing callback list with %ld entries", list->count));

    for (baranium_callback_list_entry* entry = list->start; entry != NULL; )
    {
        baranium_callback_list_entry* next = entry->next;
        free(entry);
        entry = next;
    }

    list->start = list->end = NULL;
    list->count = 0;

    free(list);
}

void baranium_callback_add(index_t id, baranium_callback_t cb, int numParams)
{
    baranium_runtime* runtime = baranium_get_runtime();

    if (runtime == NULL || id == BARANIUM_INVALID_INDEX || cb == NULL)
        return;

    baranium_callback_list* list = runtime->callbacks;
    if (list == NULL)
        return;

    baranium_callback_list_entry* newEntry = malloc(sizeof(baranium_callback_list_entry));
    if (!newEntry)
        return;
    memset(newEntry, 0, sizeof(baranium_callback_list_entry));
    newEntry->callback = cb;
    newEntry->id = id;
    newEntry->parameter_count = numParams;

    if (list->start == NULL)
    {
        newEntry->prev = NULL;
        newEntry->next = NULL;
        list->end = list->start = newEntry;
        list->count = 1;
        return;
    }

    newEntry->prev = list->end;
    newEntry->next = NULL;
    list->end->next = newEntry;

    list->end = newEntry;
    list->count++;

    return;
}

baranium_callback_list_entry* baranium_callback_find_by_id(index_t id)
{
    baranium_runtime* runtime = baranium_get_runtime();

    if (runtime == NULL || id == BARANIUM_INVALID_INDEX)
        return NULL;

    baranium_callback_list* list = runtime->callbacks;
    if (!list) return NULL;

    LOGDEBUG(stringf("Disposing callback list with %ld entries", list->count));

    for (baranium_callback_list_entry* entry = list->start; entry != NULL; )
    {
        baranium_callback_list_entry* next = entry->next;
        if (entry->id == id)
            return entry;
        entry = next;
    }

    return NULL;
}

baranium_callback_list_entry* baranium_callback_find_by_cb_ptr(baranium_callback_t cb)
{
    baranium_runtime* runtime = baranium_get_runtime();

    if (runtime == NULL || cb == NULL)
        return NULL;

    baranium_callback_list* list = runtime->callbacks;
    if (!list) return NULL;

    LOGDEBUG(stringf("Disposing callback list with %ld entries", list->count));

    for (baranium_callback_list_entry* entry = list->start; entry != NULL; )
    {
        baranium_callback_list_entry* next = entry->next;
        if (entry->callback == cb)
            return entry;
        entry = next;
    }

    return NULL;
}

void baranium_callback_remove_by_id(index_t id)
{
    baranium_runtime* runtime = baranium_get_runtime();

    if (runtime == NULL || id == BARANIUM_INVALID_INDEX)
        return;
    
    baranium_callback_list_entry* entry = baranium_callback_find_by_id(id);
    if (entry == NULL)
        return;

    baranium_callback_list_entry* prev = entry->prev;
    baranium_callback_list_entry* next = entry->next;

    free(entry);

    if (prev)
        prev->next = next;
    
    if (next)
        next->prev = prev;
}

void baranium_callback_remove_by_cb_ptr(baranium_callback_t cb)
{
    if (cb == NULL)
        return;

    baranium_callback_list_entry* entry = baranium_callback_find_by_cb_ptr(cb);
    if (entry == NULL)
        return;

    baranium_callback_list_entry* prev = entry->prev;
    baranium_callback_list_entry* next = entry->next;

    free(entry);

    if (prev)
        prev->next = next;
    
    if (next)
        next->prev = prev;
}
