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
    BaraniumVariableType_Void = 0,
    BaraniumVariableType_Object = 1,
    BaraniumVariableType_String = 2,
    BaraniumVariableType_Float = 3,
    BaraniumVariableType_Bool = 4,
    BaraniumVariableType_Int = 5,
    BaraniumVariableType_Uint = 6,
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

/**
 * @brief Check if two variable types are interchangable
 * 
 * @param target Target variable type
 * @param type Type of the second variable
 * @returns Whether the types are interchangable or not
 */
BARANIUMAPI int baranium_variable_type_size_interchangable(enum BaraniumVariableType target, enum BaraniumVariableType type);

/**
 * @brief Convert given variable into a string based on the type
 * 
 * @param type Variable type
 * @param value Variable value pointer
 * @returns The string representation of the variable value
 */
BARANIUMAPI char* baranium_variable_stringify(enum BaraniumVariableType type, void* value);

#ifdef __cplusplus
}
#endif

#endif