#include "baranium/variable.h"
#include <baranium/backend/varmath.h>
#include <baranium/field.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>

void baranium_field_set_value(baranium_field* field, void* value, size_t size, baranium_variable_type_t type)
{
    if (field == NULL || size == 0 || type == BARANIUM_VARIABLE_TYPE_INVALID || type == BARANIUM_VARIABLE_TYPE_VOID)
        return;

    baranium_compiled_variable compiled;
    compiled.size = size;
    compiled.type = type;
    if (type == BARANIUM_VARIABLE_TYPE_STRING)
    {
        compiled.value.ptr = malloc(size);
            if (compiled.value.ptr == NULL)
                return;
    }

    if (type == BARANIUM_VARIABLE_TYPE_STRING)
        memcpy(compiled.value.ptr, value, size);
    else
        memcpy(&compiled.value.num64, value, size);

    if (type != field->type)
        baranium_compiled_variable_convert_to_type(&compiled, field->type);

    if (field->type == BARANIUM_VARIABLE_TYPE_STRING)
        free(field->value.ptr);
    field->size = compiled.size;
    field->type = compiled.type;
    field->value = compiled.value;
}

baranium_value_t baranium_field_get_value(baranium_field* field, baranium_variable_type_t outputType)
{
    if (field == NULL || outputType == BARANIUM_VARIABLE_TYPE_INVALID || outputType == BARANIUM_VARIABLE_TYPE_VOID)
        return (baranium_value_t){0};
    
    if (field->type == outputType)
        return field->value;

    baranium_compiled_variable compiled;
    compiled.size = field->size;
    compiled.type = field->type;

    if (outputType == BARANIUM_VARIABLE_TYPE_STRING)
    {
        compiled.value.ptr = malloc(field->size);
        if (compiled.value.ptr == NULL)
            return (baranium_value_t){0};
    }

    if (field->type == BARANIUM_VARIABLE_TYPE_STRING)
        memcpy(compiled.value.ptr, field->value.ptr, field->size);
    else
        compiled.value = field->value;

    baranium_compiled_variable_convert_to_type(&compiled, outputType);

    return compiled.value;
}

void baranium_field_dispose(baranium_field* field)
{
    if (field == NULL)
        return;

    if (field->type == BARANIUM_VARIABLE_TYPE_STRING && field->value.ptr != NULL)
        free(field->value.ptr);

    free(field);
}
