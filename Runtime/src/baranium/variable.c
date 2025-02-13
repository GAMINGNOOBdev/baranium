#include "baranium/defines.h"
#include <baranium/variable.h>
#include <baranium/logging.h>
#include <stdlib.h>
#include <memory.h>

void baranium_variable_dispose(baranium_variable* variable)
{
    if (variable == NULL)
        return;

    if (variable->type == BARANIUM_VARIABLE_TYPE_STRING && variable->value.ptr != NULL)
        free(variable->value.ptr);

    free(variable);
}

size_t baranium_variable_get_size_of_type(baranium_variable_type_t type)
{
    switch (type)
    {
        default:
        case BARANIUM_VARIABLE_TYPE_VOID:
        case BARANIUM_VARIABLE_TYPE_INVALID:
            return 0; // zero means that nothing is being stored here

        case BARANIUM_VARIABLE_TYPE_OBJECT:
            return 8; // int64_t internally

        case BARANIUM_VARIABLE_TYPE_STRING:
            return -1;// -1 should inform about storing a string, therefore it could have any length

        case BARANIUM_VARIABLE_TYPE_FLOAT:
            return 4; // floats are still 32-bit, we can change to using double-like 64-bits later

        case BARANIUM_VARIABLE_TYPE_BOOL:
            return 1; // just use a simple single-byte integer

        case BARANIUM_VARIABLE_TYPE_INT:
        case BARANIUM_VARIABLE_TYPE_UINT:
            return 4; // default 32-bit integer
    }
}

int baranium_variable_type_size_interchangable(baranium_variable_type_t target, baranium_variable_type_t type)
{
    if (target == BARANIUM_VARIABLE_TYPE_INVALID || target == BARANIUM_VARIABLE_TYPE_VOID ||
        type == BARANIUM_VARIABLE_TYPE_INVALID || type == BARANIUM_VARIABLE_TYPE_VOID)
        return  0;

    if (target == BARANIUM_VARIABLE_TYPE_OBJECT)
    {
        return  type == BARANIUM_VARIABLE_TYPE_OBJECT || type == BARANIUM_VARIABLE_TYPE_INT     ||
                type == BARANIUM_VARIABLE_TYPE_UINT   || type == BARANIUM_VARIABLE_TYPE_FLOAT;
    }

    if (target == BARANIUM_VARIABLE_TYPE_STRING) // always converts the type value into a string!
        return  1;

    if (target == BARANIUM_VARIABLE_TYPE_FLOAT)
    {
        return  type == BARANIUM_VARIABLE_TYPE_FLOAT  || type == BARANIUM_VARIABLE_TYPE_INT     ||
                type == BARANIUM_VARIABLE_TYPE_UINT   || type == BARANIUM_VARIABLE_TYPE_STRING;
    }

    if (target == BARANIUM_VARIABLE_TYPE_BOOL)
    {
        return  type == BARANIUM_VARIABLE_TYPE_BOOL   || type == BARANIUM_VARIABLE_TYPE_FLOAT   ||
                type == BARANIUM_VARIABLE_TYPE_INT    || type == BARANIUM_VARIABLE_TYPE_UINT    ||
                type == BARANIUM_VARIABLE_TYPE_OBJECT || type == BARANIUM_VARIABLE_TYPE_STRING;
    }

    if (target == BARANIUM_VARIABLE_TYPE_INT || target == BARANIUM_VARIABLE_TYPE_UINT)
    {
        return  type == BARANIUM_VARIABLE_TYPE_BOOL   || type == BARANIUM_VARIABLE_TYPE_OBJECT  ||
                type == BARANIUM_VARIABLE_TYPE_FLOAT  || type == BARANIUM_VARIABLE_TYPE_INT     ||
                type == BARANIUM_VARIABLE_TYPE_UINT   || type == BARANIUM_VARIABLE_TYPE_STRING;
    }

    return 0;
}

char* baranium_variable_stringify(baranium_variable_type_t type, baranium_value_t value)
{
    if (type == BARANIUM_VARIABLE_TYPE_STRING)
        return value.ptr;
    
    if (type == BARANIUM_VARIABLE_TYPE_BOOL)
        return value.num8 ? "true" : "false";

    if (type == BARANIUM_VARIABLE_TYPE_OBJECT)
        return (char*)stringf("%lld", value.num64);

    if (type == BARANIUM_VARIABLE_TYPE_FLOAT)
        return (char*)stringf("%f", value.numfloat);

    if (type == BARANIUM_VARIABLE_TYPE_INT)
        return (char*)stringf("%d", value.num32);

    if (type == BARANIUM_VARIABLE_TYPE_UINT)
        return (char*)stringf("%u", value.num32);

    return NULL;
}
