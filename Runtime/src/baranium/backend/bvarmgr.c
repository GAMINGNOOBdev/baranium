#include <baranium/backend/bvarmgr.h>
#include <baranium/logging.h>
#include <memory.h>
#include <stdlib.h>

void bvarmgr_n_free(bvarmgr_n* obj)
{
    if (!obj)
        return;

    if (obj->isVariable && obj->variable)
    {
        baranium_variable_dispose(obj->variable);
    }
    else
    {
        if (obj->field)
            baranium_field_dispose(obj->field);
    }
    free(obj);
}

bvarmgr* bvarmgr_init(void)
{
    bvarmgr* obj = malloc(sizeof(bvarmgr));
    if (obj == NULL) return NULL;

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
        bvarmgr_n_free(ptr);
        ptr = next;
    }

    obj->start = obj->end = NULL;
    obj->count = 0;
}

int bvarmgr_add(bvarmgr* obj, baranium_variable* var, baranium_field* field);

void bvarmgr_alloc(bvarmgr* obj, baranium_variable_type_t type, index_t id, size_t size, bool isField)
{
    if (obj == NULL)
        return;

    if (type == BARANIUM_VARIABLE_TYPE_INVALID || type == BARANIUM_VARIABLE_TYPE_VOID)
        return;

    baranium_value_t value = {0};
    if (type == BARANIUM_VARIABLE_TYPE_STRING)
    {
        value.ptr = malloc(size);
        if (value.ptr == NULL)
            return;
        memset(value.ptr, 0, size);
    }

    baranium_variable* variable = NULL;
    baranium_field* field = NULL;
    if (isField == false)
    {
        variable = malloc(sizeof(baranium_variable));
        if (variable == NULL)
            return;

        memset(variable, 0, sizeof(baranium_variable));
        variable->id= id;
        variable->type = type;
        variable->size = size;
        variable->value = value;
    }
    else
    {
        field = malloc(sizeof(baranium_field));
        if (field == NULL)
            return;

        memset(field, 0, sizeof(baranium_field));
        field->id = id;
        field->type = type;
        field->size = size;
        field->value = value;
    }

    int status = bvarmgr_add(obj, variable, field);
    if (status)
    {
        if (type == BARANIUM_VARIABLE_TYPE_STRING)
            free(value.ptr);
        LOGERROR(stringf("Could not allocate %s with id %ld and size %ld, status code 0x%2.2x", isField ? "field" : "variable", id, size, status));
        return;
    }

    LOGDEBUG(stringf("Allocated %s with id %ld and size %ld", isField ? "field" : "variable", id, size));
}

bvarmgr_n* bvarmgr_get(bvarmgr* obj, index_t id)
{
    if (!obj)
        return NULL;

    if (!obj->start)
        return NULL;

    bvarmgr_n* entry = obj->start;
    for (; entry != NULL; entry = entry->next)
    {
        if (entry->variable != NULL)
            if (entry->variable->id == id)
                break;

        if (entry->field != NULL)
            if (entry->field->id == id)
                break;
    }

    if (!entry)
        return NULL;

    return entry;
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
        if (entry->variable != NULL)
        {
            if (entry->variable->id == id)
            {
                foundEntry = entry;
                break;
            }
        }

        if (entry->field != NULL)
        {
            if (entry->field->id == id)
            {
                foundEntry = entry;
                break;
            }
        }
    }

    if (!foundEntry)
    {
        LOGERROR(stringf("Could not find variable with id %ld", id));
        return;
    }

    bvarmgr_n* prev = foundEntry->prev;
    bvarmgr_n* next = foundEntry->next;

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
    LOGDEBUG(stringf("Disposed variable with id %ld", id));
    bvarmgr_n_free(foundEntry);
}

int bvarmgr_add(bvarmgr* obj, baranium_variable* var, baranium_field* field)
{
    if (obj == NULL || (var == NULL && field == NULL))
        return 1;

    if (obj->start == NULL)
    {
        obj->start = malloc(sizeof(bvarmgr_n));
        if (!obj->start)
        {
            baranium_variable_dispose(var);
            baranium_field_dispose(field);
            return 2;
        }
        memset(obj->start, 0, sizeof(bvarmgr_n));
        obj->start->prev = NULL;
        obj->start->next = NULL;
        obj->start->variable = var;
        obj->start->field = field;
        obj->start->isVariable = field == NULL;
        obj->end = obj->start;
        obj->count = 1;
        return 0;
    }

    bvarmgr_n* newEntry = malloc(sizeof(bvarmgr_n));
    if (!newEntry)
    {
        baranium_variable_dispose(var);
        baranium_field_dispose(field);
        return 2;
    }
    memset(newEntry, 0, sizeof(bvarmgr_n));
    newEntry->prev = obj->end;
    newEntry->variable = var;
    newEntry->field = field;
    newEntry->isVariable = field == NULL;
    newEntry->next = NULL;

    obj->end->next = newEntry;

    obj->end = newEntry;
    obj->count++;

    return 0;
}
