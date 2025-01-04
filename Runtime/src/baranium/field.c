#include <baranium/backend/varmath.h>
#include <baranium/field.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>

void baranium_field_set_value(baranium_field* field, void* value, size_t size, baranium_variable_type_t type)
{
    if (field == NULL || value == NULL || size == 0 || type == VARIABLE_TYPE_INVALID || type == VARIABLE_TYPE_VOID)
        return;

    baranium_compiled_variable compiled;
    compiled.size = size;
    compiled.type = type;
    compiled.value = malloc(size);
    if (compiled.value == NULL)
        return;

    memcpy(compiled.value, value, size);

    if (type != field->Type)
        baranium_compiled_variable_convert_to_type(&compiled, field->Type);

    free(field->Value);
    field->Size = compiled.size;
    field->Type = compiled.type;
    field->Value = compiled.value;
}

void baranium_field_get_value(baranium_field* field, void* _out, baranium_variable_type_t outputType)
{
    if (field == NULL || _out == NULL || outputType == VARIABLE_TYPE_INVALID || outputType == VARIABLE_TYPE_VOID)
        return;
    
    if (field->Type == outputType)
    {
        memcpy(_out, field->Value, field->Size);
        return;
    }

    baranium_compiled_variable compiled;
    compiled.size = field->Size;
    compiled.type = field->Type;
    compiled.value = malloc(field->Size);
    if (compiled.value == NULL)
        return;

    memcpy(compiled.value, field->Value, field->Size);

    baranium_compiled_variable_convert_to_type(&compiled, outputType);

    memcpy(_out, compiled.value, compiled.size);
    free(compiled.value);
}

void baranium_field_dispose(baranium_field* field)
{
    if (field == NULL)
        return;

    if (field->Value != NULL)
        free(field->Value);

    free(field);
}
