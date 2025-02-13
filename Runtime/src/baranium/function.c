#include "baranium/defines.h"
#include <baranium/backend/varmath.h>
#include <baranium/cpu/bstack.h>
#include <baranium/function.h>
#include <baranium/variable.h>
#include <baranium/runtime.h>
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

    if (function->return_type == BARANIUM_VARIABLE_TYPE_STRING && function->return_value.ptr != NULL)
        free(function->return_value.ptr);

    if (function->data)
        free(function->data);

    free(function);
}

void baranium_function_call(baranium_function* function, baranium_value_t* dataptr, baranium_variable_type_t* datatypes, int numData)
{
    baranium_runtime* runtime = baranium_get_context();

    if (!runtime || !function)
        return;
    
    if (runtime->cpu->kill_triggered)
        return;

    if (function->parameter_count != numData && numData != -1)
        return;

    bstack_push(runtime->function_stack, (uint64_t)runtime->cpu->bus->data_holder);
    bstack_push(runtime->cpu->ip_stack, runtime->cpu->ip);

    runtime->cpu->bus->data_holder = function;
    runtime->cpu->kill_triggered = 0;
    runtime->cpu->ip = 0;

    // only works if numData is greater zero anyways
    baranium_compiled_variable temp = {BARANIUM_VARIABLE_TYPE_INVALID, {0}, 0};
    for (int i = 0; i < numData; i++)
    {
        temp.type = datatypes[i];
        temp.value = dataptr[i];
        temp.size = baranium_variable_get_size_of_type(temp.type);
        if (temp.size == 0) // invalid or void type
            continue;
        if (temp.size == (size_t)-1)
            temp.size = strlen((const char*)temp.value.ptr);

        baranium_compiled_variable_push_to_stack(runtime->cpu, &temp);
    }
    memset(&temp, 0, sizeof(baranium_compiled_variable));

    while (!runtime->cpu->kill_triggered)
        bcpu_tick(runtime->cpu);

    if (function->return_type != BARANIUM_VARIABLE_TYPE_VOID && function->return_type != BARANIUM_VARIABLE_TYPE_INVALID)
    {
        baranium_compiled_variable* returnValue = baranium_compiled_variable_pop_from_stack(runtime->cpu);
        if (numData == -1)
            baranium_compiled_variable_push_to_stack(runtime->cpu, returnValue);

        function->return_value = returnValue->value;
        function->return_type = returnValue->type;

        baranium_compiled_variable_dispose(returnValue);
    }

    if (runtime->cpu->flags.FORCED_KILL)
        LOGERROR(stringf("Exited with code %ld", bstack_pop(runtime->cpu->stack)));
    else
        runtime->cpu->kill_triggered = 0;

    runtime->cpu->ip = bstack_pop(runtime->cpu->ip_stack);
    runtime->cpu->bus->data_holder = (baranium_function*)bstack_pop(runtime->function_stack);
}
