#ifndef __BARANIUM__VARIABLE_H_
#define __BARANIUM__VARIABLE_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"
#include <stddef.h>

#define BARANIUM_VARIABLE_TYPE_INVALID   (baranium_variable_type_t)-1
#define BARANIUM_VARIABLE_TYPE_VOID      (baranium_variable_type_t)0
#define BARANIUM_VARIABLE_TYPE_OBJECT    (baranium_variable_type_t)1
#define BARANIUM_VARIABLE_TYPE_STRING    (baranium_variable_type_t)2
#define BARANIUM_VARIABLE_TYPE_FLOAT     (baranium_variable_type_t)3
#define BARANIUM_VARIABLE_TYPE_BOOL      (baranium_variable_type_t)4
#define BARANIUM_VARIABLE_TYPE_INT       (baranium_variable_type_t)5
#define BARANIUM_VARIABLE_TYPE_UINT      (baranium_variable_type_t)6

typedef uint8_t baranium_variable_type_t;

typedef struct
{
    baranium_variable_type_t type;
    size_t size;
    baranium_value_t value;
    index_t id;
} baranium_variable;

/**
 * @brief Dispose a variable
 * 
 * @param var The variable to dispose
 */
BARANIUMAPI void baranium_variable_dispose(baranium_variable* var);

/**
 * @brief Get the size for a specific variable type
 * 
 * @param type Variable type
 * 
 * @returns The size of the variable type
 */
BARANIUMAPI size_t baranium_variable_get_size_of_type(baranium_variable_type_t type);

/**
 * @brief Check if two variable types are interchangable
 * 
 * @param target Target variable type
 * @param type Type of the second variable
 * @returns Whether the types are interchangable or not
 */
BARANIUMAPI int baranium_variable_type_size_interchangable(baranium_variable_type_t target, baranium_variable_type_t type);

/**
 * @brief Convert given variable into a string based on the type
 * 
 * @param type Variable type
 * @param value Variable value pointer
 * @returns The string representation of the variable value
 */
BARANIUMAPI char* baranium_variable_stringify(baranium_variable_type_t type, baranium_value_t value);

#ifdef __cplusplus
}
#endif

#endif
