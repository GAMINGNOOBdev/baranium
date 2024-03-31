#ifndef __BARANIUM__FUNCTION_H_
#define __BARANIUM__FUNCTION_H_ 1

#include "defines.h"
#include "variable.h"

typedef struct BaraniumFunction
{
    size_t InstructionCount;
    size_t ParameterCount;

    enum BaraniumVariableType ReturnType;
    void* ReturnValue;

    void* Instructions;
    BaraniumVariable* Parameters;
} BaraniumFunction;

#endif