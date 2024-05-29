#ifndef __BARANIUM__FUNCTION_H_
#define __BARANIUM__FUNCTION_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"
#include "variable.h"

typedef struct BaraniumFunction
{
    size_t DataSize;
    size_t ParameterCount;

    index_t ID;

    enum BaraniumVariableType ReturnType;
    void* ReturnValue;

    void* Data;
    BaraniumVariable* Parameters;
} BaraniumFunction;

/**
 * @brief Dispose a function
 * 
 * @param var The function to dispose
 */
BARANIUMAPI void baranium_function_dispose(BaraniumFunction* var);

BARANIUMAPI void baranium_function_call(BaraniumRuntime* runtime, BaraniumFunction* function, ...);

#ifdef __cplusplus
}
#endif

#endif