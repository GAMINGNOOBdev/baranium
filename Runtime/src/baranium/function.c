#include <baranium/cpu/bstack.h>
#include <baranium/function.h>
#include <baranium/logging.h>
#include <baranium/bcpu.h>
#include <stdarg.h>
#include <stdlib.h>

void baranium_function_dispose(BaraniumFunction* function)
{
    if (function == NULL)
        return;

    if (function->Data)
        free(function->Data);

    free(function);
}

void baranium_function_call(BaraniumRuntime* runtime, BaraniumFunction* function, ...)
{
    ///TODO: call the function + parse args and put them into the parameters of the function

    if (!runtime || !function)
        return;
    
    if (runtime->cpu->killTriggered)
        return;

    bstack_push(runtime->functionStack, (uint64_t)runtime->cpu->bus->dataHolder);
    bstack_push(runtime->cpu->ip_stack, runtime->cpu->IP);

    runtime->cpu->bus->dataHolder = function;
    runtime->cpu->killTriggered = 0;
    runtime->cpu->IP = 0;

    while (!runtime->cpu->killTriggered)
        bcpu_tick(runtime->cpu);

    if (runtime->cpu->flags.FORCED_KILL)
        LOGERROR(stringf("Exited with code %ld", bstack_pop(runtime->cpu->stack)));

    runtime->cpu->IP = bstack_pop(runtime->cpu->ip_stack);
    runtime->cpu->bus->dataHolder = (BaraniumFunction*)bstack_pop(runtime->functionStack);
}