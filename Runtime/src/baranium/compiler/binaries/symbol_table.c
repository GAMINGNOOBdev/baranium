#include <baranium/compiler/binaries/symbol_table.h>
#include <baranium/defines.h>
#include <stdlib.h>
#include <memory.h>

void baranium_symbol_table_init(baranium_symbol_table* table)
{
    if (table == NULL)
        return;

    memset(table, 0, sizeof(baranium_symbol_table));
}

void baranium_symbol_table_dispose(baranium_symbol_table* table)
{
    if (table == NULL || table->data == NULL || table->count == 0)
        return;

    free(table->data);
    memset(table, 0, sizeof(baranium_symbol_table));
}

void baranium_symbol_table_clear(baranium_symbol_table* table)
{
    if (table == NULL || table->data == NULL || table->count == 0)
        return;

    table->count = 0;
}

index_t baranium_symbol_table_lookup(baranium_symbol_table* table, const char* name)
{
    if (table == NULL || name == NULL || table->data == NULL || table->count == 0)
        return BARANIUM_INVALID_INDEX;

    for (size_t i = 0; i < table->count; i++)
        if (strcmp(table->data[i].name, name) == 0)
            return table->data[i].id;

    return BARANIUM_INVALID_INDEX;
}

const char* baranium_symbol_table_lookup_name(baranium_symbol_table* table, index_t id)
{
    if (table == NULL || id == BARANIUM_INVALID_INDEX || table->data == NULL || table->count)
        return NULL;

    for (size_t i = 0; i < table->count; i++)
        if (table->data[i].id == id)
            return table->data[i].name;

    return NULL;
}

void baranium_symbol_table_add(baranium_symbol_table* table, baranium_variable_token* var)
{
    if (table == NULL || var == NULL)
        return;

    baranium_symbol_table_add_from_name_and_id(table, var->base.name, var->base.id);
}

void baranium_symbol_table_add_from_name_and_id(baranium_symbol_table* table, const char* name, index_t id)
{
    if (table == NULL || name == NULL || id == BARANIUM_INVALID_INDEX)
        return;

    if (baranium_symbol_table_lookup_name(table, id) != NULL)
        return;

    if (table->count + 1 >= table->buffer_size)
    {
        table->buffer_size += BARANIUM_SYMBOL_TABLE_BUFFER_SIZE;
        table->data = realloc(table->data, sizeof(baranium_symbol_table_entry)*table->buffer_size);
    }

    table->data[table->count] = (baranium_symbol_table_entry){.id = id, .name = name};
    table->count++;
}

void baranium_symbol_table_remove(baranium_symbol_table* table, baranium_variable_token* var)
{
    if (table == NULL || var == NULL || var->base.id == BARANIUM_INVALID_INDEX || table->data == NULL || table->count)
        return;

    size_t index = -1;
    for (size_t i = 0; i < table->count; i++)
    {
        if (table->data[i].id == var->base.id)
        {
            index = i;
            break;
        }
    }

    if (index == (size_t)-1)
        return;

    for (size_t idx = index; idx < table->count-1; idx++)
    {
        baranium_symbol_table_entry tmp = {.id = BARANIUM_INVALID_INDEX, .name = NULL};
        size_t tmpindex = idx+1;
        if (tmpindex < table->count)
            tmp = table->data[tmpindex];
        table->data[idx] = tmp;
    }
    table->count--;
}
