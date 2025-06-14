#include "baranium/defines.h"
#include <baranium/variable.h>
#include <baranium/logging.h>
#include <stdlib.h>
#include <memory.h>

const char* baranium_variable_type_to_string(baranium_variable_type_t type)
{
    if (type == BARANIUM_VARIABLE_TYPE_VOID)
        return "Void";
    if (type == BARANIUM_VARIABLE_TYPE_OBJECT)
        return "Object";
    if (type == BARANIUM_VARIABLE_TYPE_STRING)
        return "String";
    if (type == BARANIUM_VARIABLE_TYPE_FLOAT)
        return "Float";
    if (type == BARANIUM_VARIABLE_TYPE_BOOL)
        return "Bool";
    if (type == BARANIUM_VARIABLE_TYPE_INT32)
        return "Int32";
    if (type == BARANIUM_VARIABLE_TYPE_UINT32)
        return "Uint32";
    if (type == BARANIUM_VARIABLE_TYPE_DOUBLE)
        return "Double";
    if (type == BARANIUM_VARIABLE_TYPE_INT8)
        return "Int8";
    if (type == BARANIUM_VARIABLE_TYPE_UINT8)
        return "Uint8";
    if (type == BARANIUM_VARIABLE_TYPE_INT16)
        return "Int16";
    if (type == BARANIUM_VARIABLE_TYPE_UINT16)
        return "Uint16";
    if (type == BARANIUM_VARIABLE_TYPE_INT64)
        return "Int64";
    if (type == BARANIUM_VARIABLE_TYPE_UINT64)
        return "Uint64";

    return "Invalid";
}

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
        return sizeof(int64_t); // int64_t internally

    case BARANIUM_VARIABLE_TYPE_STRING:
        return -1; // -1 should inform about storing a string, therefore it could have any length

    case BARANIUM_VARIABLE_TYPE_FLOAT:
        return sizeof(float);
    case BARANIUM_VARIABLE_TYPE_DOUBLE:
        return sizeof(double);

    case BARANIUM_VARIABLE_TYPE_INT32:
    case BARANIUM_VARIABLE_TYPE_UINT32:
        return sizeof(uint32_t); // default 32-bit integer
    
    case BARANIUM_VARIABLE_TYPE_BOOL:
    case BARANIUM_VARIABLE_TYPE_INT8:
    case BARANIUM_VARIABLE_TYPE_UINT8:
        return sizeof(uint8_t);

    case BARANIUM_VARIABLE_TYPE_INT16:
    case BARANIUM_VARIABLE_TYPE_UINT16:
        return sizeof(uint16_t);

    case BARANIUM_VARIABLE_TYPE_INT64:
    case BARANIUM_VARIABLE_TYPE_UINT64:
        return sizeof(uint64_t);
    }
}

int baranium_variable_type_size_interchangable(baranium_variable_type_t target, baranium_variable_type_t type)
{
    if (target == BARANIUM_VARIABLE_TYPE_INVALID || target == BARANIUM_VARIABLE_TYPE_VOID ||
        type == BARANIUM_VARIABLE_TYPE_INVALID || type == BARANIUM_VARIABLE_TYPE_VOID)
        return 0;

    if (target == BARANIUM_VARIABLE_TYPE_OBJECT)
    {
        return  type != BARANIUM_VARIABLE_TYPE_BOOL && type != BARANIUM_VARIABLE_TYPE_FLOAT &&
                type != BARANIUM_VARIABLE_TYPE_DOUBLE;
    }

    if (target == BARANIUM_VARIABLE_TYPE_STRING) // always converts the type value into a string!
        return 1;

    if (target == BARANIUM_VARIABLE_TYPE_FLOAT || target == BARANIUM_VARIABLE_TYPE_DOUBLE)
    {
        return  type != BARANIUM_VARIABLE_TYPE_BOOL && type != BARANIUM_VARIABLE_TYPE_OBJECT;
    }

    if (target == BARANIUM_VARIABLE_TYPE_BOOL) // anything can be a bool (only true if a positive value is present)
    {
        return 1;
    }

    if (target == BARANIUM_VARIABLE_TYPE_INT32 || target == BARANIUM_VARIABLE_TYPE_UINT32 ||
        target == BARANIUM_VARIABLE_TYPE_INT16 || target == BARANIUM_VARIABLE_TYPE_UINT16 ||
        target == BARANIUM_VARIABLE_TYPE_INT8 || target == BARANIUM_VARIABLE_TYPE_UINT8 ||
        target == BARANIUM_VARIABLE_TYPE_INT64 || target == BARANIUM_VARIABLE_TYPE_UINT64)
    {
        return 1;
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

    if (type == BARANIUM_VARIABLE_TYPE_INT32)
        return (char*)stringf("%d", value.num32);

    if (type == BARANIUM_VARIABLE_TYPE_UINT32)
        return (char*)stringf("%u", value.num32);

    if (type == BARANIUM_VARIABLE_TYPE_DOUBLE)
        return (char*)stringf("%f", value.numdouble);

    if (type == BARANIUM_VARIABLE_TYPE_INT8)
        return (char*)stringf("%d", value.snum8);

    if (type == BARANIUM_VARIABLE_TYPE_UINT8)
        return (char*)stringf("%u", value.num8);

    if (type == BARANIUM_VARIABLE_TYPE_INT16)
        return (char*)stringf("%d", value.snum16);

    if (type == BARANIUM_VARIABLE_TYPE_UINT16)
        return (char*)stringf("%u", value.num16);

    if (type == BARANIUM_VARIABLE_TYPE_INT64)
        return (char*)stringf("%lld", value.snum64);

    if (type == BARANIUM_VARIABLE_TYPE_UINT64)
        return (char*)stringf("%llu", value.num64);

    return NULL;
}
