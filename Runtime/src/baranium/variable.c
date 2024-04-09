#include <baranium/variable.h>

void baranium_variable_dispose(BaraniumVariable* variable)
{
    if (variable == NULL)
        return;

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

        case BaraniumVariableType_GameObject:
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
