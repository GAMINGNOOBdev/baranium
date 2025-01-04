#include <baranium/backend/varmath.h>
#include <baranium/cpu/bstack.h>
#include <baranium/function.h>
#include <baranium/variable.h>
#include <baranium/logging.h>
#include <baranium/bcpu.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>
#include <stdlib.h>

void baranium_function_dispose(baranium_function* function)
{
    if (function == NULL)
        return;

    if (function->ReturnValue != NULL)
        free(function->ReturnValue);

    if (function->Data)
        free(function->Data);

    free(function);
}

void baranium_function_call(baranium_runtime* runtime, baranium_function* function, void** dataptr, baranium_variable_type_t* datatypes, int numData)
{
    if (!runtime || !function)
        return;
    
    if (runtime->cpu->killTriggered)
        return;

    if (function->ParameterCount != numData && numData != -1)
        return;

    bstack_push(runtime->functionStack, (uint64_t)runtime->cpu->bus->dataHolder);
    bstack_push(runtime->cpu->ip_stack, runtime->cpu->IP);

    runtime->cpu->bus->dataHolder = function;
    runtime->cpu->killTriggered = 0;
    runtime->cpu->IP = 0;

    // only works if numData is greater zero anyways
    baranium_compiled_variable temp = {VARIABLE_TYPE_INVALID, NULL, 0};
    for (int i = 0; i < numData; i++)
    {
        temp.type = datatypes[i];
        temp.value = dataptr[i];
        temp.size = baranium_variable_get_size_of_type(temp.type);
        if (temp.size == 0) // invalid or void type
            continue;
        if (temp.size == (size_t)-1)
            temp.size = strlen((const char*)temp.value);

        baranium_compiled_variable_push_to_stack(runtime->cpu, &temp);
    }
    memset(&temp, 0, sizeof(baranium_compiled_variable));

    while (!runtime->cpu->killTriggered)
        bcpu_tick(runtime->cpu);

    if (function->ReturnType != VARIABLE_TYPE_VOID && function->ReturnType != VARIABLE_TYPE_INVALID)
    {
        baranium_compiled_variable* returnValue = baranium_compiled_variable_pop_from_stack(runtime->cpu);
        if (numData == -1)
            baranium_compiled_variable_push_to_stack(runtime->cpu, returnValue);

        function->ReturnValue = returnValue->value;
        function->ReturnType = returnValue->type;

        baranium_compiled_variable_dispose(returnValue);
    }

    if (runtime->cpu->flags.FORCED_KILL)
        LOGERROR(stringf("Exited with code %ld", bstack_pop(runtime->cpu->stack)));
    else
        runtime->cpu->killTriggered = 0;

    runtime->cpu->IP = bstack_pop(runtime->cpu->ip_stack);
    runtime->cpu->bus->dataHolder = (baranium_function*)bstack_pop(runtime->functionStack);
}
