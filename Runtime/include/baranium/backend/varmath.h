#ifndef __BARANIUM__BACKEND__VARMATH_H_
#define __BARANIUM__BACKEND__VARMATH_H_

#include "../variable.h"
#include "../defines.h"
#include "../bcpu.h"

#define BARANIUM_VARIABLE_OPERATION_NONE    0x00
#define BARANIUM_VARIABLE_OPERATION_MOD     0x01
#define BARANIUM_VARIABLE_OPERATION_DIV     0x02
#define BARANIUM_VARIABLE_OPERATION_MUL     0x03
#define BARANIUM_VARIABLE_OPERATION_SUB     0x04
#define BARANIUM_VARIABLE_OPERATION_ADD     0x05
#define BARANIUM_VARIABLE_OPERATION_AND     0x06
#define BARANIUM_VARIABLE_OPERATION_OR      0x07
#define BARANIUM_VARIABLE_OPERATION_XOR     0x08
#define BARANIUM_VARIABLE_OPERATION_SHFTL   0x09
#define BARANIUM_VARIABLE_OPERATION_SHFTR   0x0A

#ifdef __cplusplus
extern "C" {
#endif

typedef struct baranium_compiled_variable
{
    baranium_variable_type_t type;
    void* value;
    size_t size;
} baranium_compiled_variable;

/**
 * @brief Pop a compiled variable from the cpu stack
 * 
 * @param cpu CPU
 * 
 * @returns Compiled variable object
 */
BARANIUMAPI baranium_compiled_variable* baranium_compiled_variable_pop_from_stack(bcpu* cpu);

/**
 * @brief Push a compiled variable to the cpu stack
 * 
 * @param cpu CPU
 * @param var Compiled variable
 */
BARANIUMAPI void baranium_compiled_variable_push_to_stack(bcpu* cpu, baranium_compiled_variable* var);

/**
 * @brief Disposes the created compiled variable object
 * 
 * @note This does NOT clear the DATA
 * 
 * @param varptr Variable object pointer
 */
BARANIUMAPI void baranium_compiled_variable_dispose(baranium_compiled_variable* varptr);

/**
 * @brief Convert a variable to a specific type
 * 
 * @note This will modify the values `var`
 * 
 * @param var Variable that will be converted
 * @param targetType Target type
 */
BARANIUMAPI void baranium_compiled_variable_convert_to_type(baranium_compiled_variable* var, baranium_variable_type_t targetType);

/**
 * @brief Combine two variable's data
 * 
 * @note The type of the left hand side will be used and modified, both parameter objects will not be disposed automatically
 * 
 * @param lhs Left hand side
 * @param rhs Right hand side
 * @param operation Type of operation to be done to the variable
 * @param type Target variable type
 */
BARANIUMAPI void baranium_compiled_variable_combine(baranium_compiled_variable* lhs, baranium_compiled_variable* rhs, uint8_t operation, baranium_variable_type_t type);

#ifdef __cplusplus
}
#endif

#endif
