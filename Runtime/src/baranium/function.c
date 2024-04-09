#include <baranium/function.h>
#include <stdarg.h>

void baranium_function_dispose(BaraniumFunction* function)
{
    if (function == NULL)
        return;

    free(function);
}

void baranium_function_call(BaraniumRuntime* runtime, BaraniumFunction* function, ...)
{
    ///TODO: call the function + parse args and put them into the parameters of the function
}