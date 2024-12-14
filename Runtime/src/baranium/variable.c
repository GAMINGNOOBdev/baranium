#include <baranium/variable.h>
#include <baranium/logging.h>
#include <stdlib.h>
#include <memory.h>

void baranium_variable_dispose(baranium_variable* variable)
{
    if (variable == NULL)
        return;

    if (variable->Value != NULL)
        free(variable->Value);

    free(variable);
}

size_t baranium_variable_get_size_of_type(baranium_variable_type_t type)
{
    switch (type)
    {
        default:
        case VARIABLE_TYPE_VOID:
        case VARIABLE_TYPE_INVALID:
            return 0; // zero means that nothing is being stored here

        case VARIABLE_TYPE_OBJECT:
            return 8; // int64_t internally

        case VARIABLE_TYPE_STRING:
            return -1;// -1 should inform about storing a string, therefore it could have any length

        case VARIABLE_TYPE_FLOAT:
            return 4; // floats are still 32-bit, we can change to using double-like 64-bits later

        case VARIABLE_TYPE_BOOL:
            return 1; // just use a simple single-byte integer

        case VARIABLE_TYPE_INT:
        case VARIABLE_TYPE_UINT:
            return 4; // default 32-bit integer
    }
}

int baranium_variable_type_size_interchangable(baranium_variable_type_t target, baranium_variable_type_t type)
{
    if (target == VARIABLE_TYPE_INVALID || target == VARIABLE_TYPE_VOID ||
        type == VARIABLE_TYPE_INVALID || type == VARIABLE_TYPE_VOID)
        return  0;

    if (target == VARIABLE_TYPE_OBJECT)
    {
        return  type == VARIABLE_TYPE_OBJECT || type == VARIABLE_TYPE_INT     ||
                type == VARIABLE_TYPE_UINT   || type == VARIABLE_TYPE_FLOAT;
    }

    if (target == VARIABLE_TYPE_STRING) // always converts the type value into a string!
        return  1;

    if (target == VARIABLE_TYPE_FLOAT)
    {
        return  type == VARIABLE_TYPE_FLOAT  || type == VARIABLE_TYPE_INT     ||
                type == VARIABLE_TYPE_UINT;
    }

    if (target == VARIABLE_TYPE_BOOL)
    {
        return  type == VARIABLE_TYPE_BOOL   || type == VARIABLE_TYPE_FLOAT   ||
                type == VARIABLE_TYPE_INT    || type == VARIABLE_TYPE_UINT    ||
                type == VARIABLE_TYPE_OBJECT;
    }

    if (target == VARIABLE_TYPE_INT || target == VARIABLE_TYPE_UINT)
    {
        return  type == VARIABLE_TYPE_BOOL   || type == VARIABLE_TYPE_OBJECT  ||
                type == VARIABLE_TYPE_FLOAT  || type == VARIABLE_TYPE_INT     ||
                type == VARIABLE_TYPE_UINT;
    }

    return 0;
}

char* baranium_variable_stringify(baranium_variable_type_t type, void* value)
{
    if (type == VARIABLE_TYPE_STRING)
        return value;
    
    if (type == VARIABLE_TYPE_BOOL)
    {
        uint64_t val = *((uint64_t*)value);
        return val ? "true" : "false";
    }

    if (type == VARIABLE_TYPE_OBJECT)
    {
        int64_t val = *((uint64_t*)value);
        return stringf("%lld", val);
    }

    if (type == VARIABLE_TYPE_FLOAT)
    {
        uint32_t valInt = *((uint64_t*)value);
        float val = *((float*)&valInt);
        return stringf("%f", val);
    }

    if (type == VARIABLE_TYPE_INT)
    {
        int32_t val = *((uint64_t*)value);
        return stringf("%d", val);
    }

    if (type == VARIABLE_TYPE_UINT)
    {
        uint32_t val = *((uint64_t*)value);
        return stringf("%u", val);
    }

    return NULL;
}
