#ifndef __BARANIUM__FUNCTION_H_
#define __BARANIUM__FUNCTION_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"
#include "variable.h"

// forward declaration needed to create a pointer
struct baranium_script;

typedef struct baranium_function
{
    size_t DataSize;
    size_t ParameterCount;

    index_t ID;

    baranium_variable_type_t ReturnType;
    void* ReturnValue;

    void* Data;
    baranium_variable* Parameters;

    struct baranium_script* Script;
} baranium_function;

/**
 * @brief Dispose a function
 * 
 * @param var The function to dispose
 */
BARANIUMAPI void baranium_function_dispose(baranium_function* var);

BARANIUMAPI void baranium_function_call(baranium_runtime* runtime, baranium_function* function, ...);

#ifdef __cplusplus
}
#endif

#endif
