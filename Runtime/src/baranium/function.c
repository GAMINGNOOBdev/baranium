#include <baranium/backend/varmath.h>
#include <baranium/backend/errors.h>
#include <baranium/cpu/bstack.h>
#include <baranium/function.h>
#include <baranium/variable.h>
#include <baranium/defines.h>
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

    if (function->return_data.type == BARANIUM_VARIABLE_TYPE_STRING && function->return_data.value.ptr != NULL)
        free(function->return_data.value.ptr);

    if (function->data)
        free(function->data);

    free(function);
}

void baranium_function_call(baranium_function* function, baranium_function_call_data_t data)
{
    baranium_runtime* runtime = baranium_get_runtime();

    if (!runtime || !function)
        return;
    
    if (runtime->cpu->kill_triggered)
        return;

    if (function->parameter_count != data.count && data.count != -1)
        return;

    bstack_push(runtime->function_stack, (uint64_t)runtime->cpu->bus->data_holder);
    bstack_push(runtime->cpu->ip_stack, runtime->cpu->ip);

    runtime->cpu->bus->data_holder = function;
    runtime->cpu->kill_triggered = 0;
    runtime->cpu->ip = 0;

    // only works if data.count is greater zero anyways
    baranium_compiled_variable temp = {BARANIUM_VARIABLE_TYPE_INVALID, {0}, 0};
    for (int i = 0; i < data.count; i++)
    {
        temp.type = data.types[i];
        temp.value = data.data[i];
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

    if (function->return_data.type != BARANIUM_VARIABLE_TYPE_VOID && function->return_data.type != BARANIUM_VARIABLE_TYPE_INVALID)
    {
        baranium_compiled_variable* returnValue = baranium_compiled_variable_pop_from_stack(runtime->cpu);
        if (data.count == -1)
            baranium_compiled_variable_push_to_stack(runtime->cpu, returnValue);

        function->return_data.value = returnValue->value;
        function->return_data.type = returnValue->type;
        function->return_data.array_size = returnValue->type;

        baranium_compiled_variable_dispose(returnValue);
    }

    if (runtime->cpu->flags.FORCED_KILL)
    {
        uint64_t err = bstack_pop(runtime->cpu->stack);
        LOGERROR("Stack trace:");
        for (size_t i = runtime->cpu->ip_stack->count; i >= 0; --i)
        {
            if (i == runtime->cpu->ip_stack->count-1)
            {
                LOGERROR("\t%current function called from %lld", runtime->cpu->ip_stack->stackptr[i]);
                continue;
            }

            LOGERROR("%lld called from %lld", runtime->cpu->ip_stack->stackptr[i+1], runtime->cpu->ip_stack->stackptr[i]);
        }

        LOGERROR("Exited with code %ld: %s", err, BARANIUM_ERROR_TO_STRING(err));
    }
    else
        runtime->cpu->kill_triggered = 0;

    runtime->cpu->ip = bstack_pop(runtime->cpu->ip_stack);
    runtime->cpu->bus->data_holder = (baranium_function*)bstack_pop(runtime->function_stack);
}
