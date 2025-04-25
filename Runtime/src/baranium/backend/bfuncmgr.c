#include <baranium/backend/bfuncmgr.h>
#include <baranium/library.h>
#include <baranium/logging.h>
#include <memory.h>
#include <stdlib.h>

baranium_function_manager* baranium_function_manager_init(void)
{
    baranium_function_manager* obj = malloc(sizeof(baranium_function_manager));
    if (obj == NULL) return NULL;

    memset(obj, 0, sizeof(baranium_function_manager));
    baranium_function_manager_clear(obj);

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

    if (obj->buffer == NULL) return;

    LOGDEBUG(stringf("Cleared function manager with %ld entries", obj->count));

    free(obj->buffer);
    memset(obj, 0, sizeof(baranium_function_manager));
}

int baranium_function_manager_add_entry(baranium_function_manager* obj, index_t id, baranium_script* script, baranium_library* library);

void baranium_function_manager_add(baranium_function_manager* obj, index_t id, baranium_script* script, baranium_library* library)
{
    if (obj == NULL)
        return;

    if (id == BARANIUM_INVALID_INDEX)
        return;

    baranium_function_manager_entry* func = baranium_function_manager_get_entry(obj, id);
    if (func != NULL)
        return;

    int status = baranium_function_manager_add_entry(obj, id, script, library);
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

    baranium_function_manager_entry* entry = baranium_function_manager_get_entry(obj, id);
    if (entry == NULL)
        return NULL;

    if (entry->script != NULL)
        return baranium_script_get_function_by_id(entry->script, entry->id);
    else if (entry->library != NULL)
        return baranium_library_get_function_by_id(entry->library, entry->id);

    return NULL;
}

baranium_function_manager_entry* baranium_function_manager_get_entry(baranium_function_manager* obj, index_t id)
{
    if (!obj)
        return NULL;

    if (obj->buffer == NULL)
        return NULL;

    for (uint64_t i = 0; i < obj->count; i++)
    {
        if (obj->buffer[i].id == id)
            return &obj->buffer[i];
    }

    return NULL;
}

int baranium_function_manager_get_entry_index(baranium_function_manager* obj, index_t id)
{
    if (!obj)
        return -1;

    if (obj->buffer == NULL)
        return -1;

    for (uint64_t i = 0; i < obj->count; i++)
    {
        if (obj->buffer[i].id == id)
            return i;
    }

    return -1;
}

void baranium_function_manager_remove(baranium_function_manager* obj, index_t id)
{
    if (obj == NULL)
        return;

    if (obj->buffer == NULL)
        return;

    int index = baranium_function_manager_get_entry_index(obj, id);
    if (index == -1)
    {
        LOGERROR(stringf("Could not find function with id %ld", id));
        return;
    }

    if ((size_t)index == obj->count-1)
    {
        memset(&obj->buffer[index], 0, sizeof(baranium_function_manager_entry));
        obj->count--;
        return;
    }

    LOGDEBUG(stringf("Disposed function with id %ld", id));
    memset(&obj->buffer[index], 0, sizeof(baranium_function_manager_entry));
    memmove(&obj->buffer[index], &obj->buffer[index+1], sizeof(baranium_function_manager_entry)*(obj->count - index - 1));

    obj->count--;
}

int baranium_function_manager_add_entry(baranium_function_manager* obj, index_t id, baranium_script* script, baranium_library* library)
{
    if (obj == NULL || id == BARANIUM_INVALID_INDEX || (script == NULL && library == NULL))
        return 1;

    baranium_function_manager_entry entry = {
        .id = id,
        .script = script,
        .library = library,
    };

    if (obj->buffer_size <= obj->count+1)
    {
        obj->buffer_size += BARANIUM_FUNCTION_MANAGER_BUFFER_SIZE;
        obj->buffer = realloc(obj->buffer, sizeof(baranium_function_manager_entry)*obj->buffer_size);
    }

    memcpy(&obj->buffer[obj->count], &entry, sizeof(baranium_function_manager_entry));
    obj->count++;

    return 0;
}
