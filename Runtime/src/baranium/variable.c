#include <baranium/variable.h>
#include <baranium/logging.h>
#include <stdlib.h>
#include <memory.h>

void baranium_variable_dispose(BaraniumVariable* variable)
{
    if (variable == NULL)
        return;

    if (variable->Value != NULL)
        free(variable->Value);

    free(variable);
}

size_t baranium_variable_get_size_of_type(enum BaraniumVariableType type)
{
    switch (type)
    {
        default:
        case BaraniumVariableType_Void:
        case BaraniumVariableType_Invalid:
            return 0; // zero means that nothing is being stored here

        case BaraniumVariableType_Object:
            return 8; // int64_t internally

        case BaraniumVariableType_String:
            return -1;// -1 should inform about storing a string, therefore it could have any length

        case BaraniumVariableType_Float:
            return 4; // floats are still 32-bit, we can change to using double-like 64-bits later

        case BaraniumVariableType_Bool:
            return 1; // just use a simple single-byte integer

        case BaraniumVariableType_Int:
        case BaraniumVariableType_Uint:
            return 4; // default 32-bit integer
    }
}

int baranium_variable_type_size_interchangable(enum BaraniumVariableType target, enum BaraniumVariableType type)
{
    if (target == BaraniumVariableType_Invalid || target == BaraniumVariableType_Void ||
        type == BaraniumVariableType_Invalid || type == BaraniumVariableType_Void)
        return  0;

    if (target == BaraniumVariableType_Object)
    {
        return  type == BaraniumVariableType_Object || type == BaraniumVariableType_Int     ||
                type == BaraniumVariableType_Uint   || type == BaraniumVariableType_Float;
    }

    if (target == BaraniumVariableType_String) // always converts the type value into a string!
        return  1;

    if (target == BaraniumVariableType_Float)
    {
        return  type == BaraniumVariableType_Float  || type == BaraniumVariableType_Int     ||
                type == BaraniumVariableType_Uint;
    }

    if (target == BaraniumVariableType_Bool)
    {
        return  type == BaraniumVariableType_Bool   || type == BaraniumVariableType_Float   ||
                type == BaraniumVariableType_Int    || type == BaraniumVariableType_Uint    ||
                type == BaraniumVariableType_Object;
    }

    if (target == BaraniumVariableType_Int || target == BaraniumVariableType_Uint)
    {
        return  type == BaraniumVariableType_Bool   || type == BaraniumVariableType_Object  ||
                type == BaraniumVariableType_Float  || type == BaraniumVariableType_Int     ||
                type == BaraniumVariableType_Uint;
    }

    return 0;
}

char* baranium_variable_stringify(enum BaraniumVariableType type, void* value)
{
    if (type == BaraniumVariableType_String)
        return value;
    
    if (type == BaraniumVariableType_Bool)
    {
        uint64_t val = *((uint64_t*)value);
        return val ? "true" : "false";
    }

    if (type == BaraniumVariableType_Object)
    {
        int64_t val = *((uint64_t*)value);
        return stringf("%lld", val);
    }

    if (type == BaraniumVariableType_Float)
    {
        uint32_t valInt = *((uint64_t*)value);
        float val = *((float*)&valInt);
        return stringf("%f", val);
    }

    if (type == BaraniumVariableType_Int)
    {
        int32_t val = *((uint64_t*)value);
        return stringf("%d", val);
    }

    if (type == BaraniumVariableType_Uint)
    {
        uint32_t val = *((uint64_t*)value);
        return stringf("%u", val);
    }

    return NULL;
}