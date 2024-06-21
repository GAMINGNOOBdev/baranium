#ifndef __BARANIUM__VARIABLE_H_
#define __BARANIUM__VARIABLE_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"
#include <stddef.h>

enum BaraniumVariableType
{
    BaraniumVariableType_Invalid = -1,
    BaraniumVariableType_Void,
    BaraniumVariableType_Object,
    BaraniumVariableType_String,
    BaraniumVariableType_Float,
    BaraniumVariableType_Bool,
    BaraniumVariableType_Int,
    BaraniumVariableType_Uint,
};

typedef struct BaraniumVariable
{
    enum BaraniumVariableType Type;
    size_t Size;
    void* Value;
    index_t ID;
} BaraniumVariable;

/**
 * @brief Dispose a variable
 * 
 * @param var The variable to dispose
 */
BARANIUMAPI void baranium_variable_dispose(BaraniumVariable* var);

/**
 * @brief Get the size for a specific variable type
 * 
 * @param type Variable type
 * 
 * @returns The size of the variable type
 */
BARANIUMAPI size_t baranium_variable_get_size_of_type(enum BaraniumVariableType type);

#ifdef __cplusplus
}
#endif

#endif