#ifndef __BARANIUM__FUNCTION_H_
#define __BARANIUM__FUNCTION_H_ 1

#include "defines.h"
#include "variable.h"

typedef struct BaraniumFunction
{
    size_t InstructionCount;
    size_t ParameterCount;

    index_t ID;

    enum BaraniumVariableType ReturnType;
    void* ReturnValue;

    void* Instructions;
    BaraniumVariable* Parameters;
} BaraniumFunction;

/**
 * @brief Dispose a function
 * 
 * @param var The function to dispose
 */
BARANIUMAPI void baranium_function_dispose(BaraniumFunction* var);

BARANIUMAPI void baranium_function_call(BaraniumRuntime* runtime, BaraniumFunction* function, ...);

#endif