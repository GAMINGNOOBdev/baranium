#include <baranium/cpu/bcpu_opcodes.h>
#include <baranium/cpu/bstack.h>
#include <baranium/runtime.h>
#include <baranium/logging.h>
#include <baranium/bcpu.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

static BaraniumRuntime* current_active_runtime = NULL;

BaraniumRuntime* baranium_init()
{
    bcpu_opcodes_init();

    BaraniumRuntime* runtimeHandle = malloc(sizeof(BaraniumRuntime));
    if (!runtimeHandle)
        return NULL;

    memset(runtimeHandle, 0, sizeof(BaraniumRuntime));

    runtimeHandle->cpu = bcpu_init(runtimeHandle);
    runtimeHandle->functionStack = bstack_init();
    runtimeHandle->varmgr = bvarmgr_init();
    return runtimeHandle;
}

void baranium_set_context(BaraniumRuntime* runtimeContext)
{
    current_active_runtime = runtimeContext;
}

BaraniumRuntime* baranium_get_context()
{
    return current_active_runtime;
}

void baranium_cleanup(BaraniumRuntime* runtime)
{
    if (runtime == NULL) return;

    if (!(runtime->start == NULL && runtime->end == NULL))
    {
        BaraniumHandle* handle = runtime->start;
        BaraniumHandle* next = handle->next;
        while (handle != NULL)
        {
            next = handle->next;

            if (handle->file)
                fclose(handle->file);

            free(handle);
            handle = next;
        }
    }

    bcpu_dispose(runtime->cpu);
    bstack_dispose(runtime->functionStack);
    bvarmgr_dispose(runtime->varmgr);
    free(runtime);
}

BaraniumHandle* baranium_open_handle(const char* source)
{
    if (current_active_runtime == NULL)
        return NULL;

    FILE* file = fopen(source, "rb");
    if (file == NULL)
    {
        LOGERROR(stringf("File '%s' not found", source));
        return NULL;
    }

    BaraniumHandle* handle = (BaraniumHandle*)malloc(sizeof(BaraniumHandle));
    if (!handle)
        return NULL;

    memset(handle, 0, sizeof(BaraniumHandle));
    handle->file = file;

    if (current_active_runtime->start == NULL)
    {
        current_active_runtime->start = current_active_runtime->end = handle;
        current_active_runtime->openHandles = 1;
        goto end;
    }

    if (current_active_runtime->start == current_active_runtime->end)
    {
        handle->prev = current_active_runtime->start;
        current_active_runtime->start->next = current_active_runtime->end = handle;
        current_active_runtime->openHandles++;
        goto end;
    }

    handle->prev = current_active_runtime->end;
    current_active_runtime->end->next = handle;
    current_active_runtime->end = handle;
    current_active_runtime->openHandles++;

end:
    return handle;
}

void baranium_close_handle(BaraniumHandle* handle)
{
    if (current_active_runtime == NULL || handle == NULL)
        return;

    BaraniumHandle* currentHandle = current_active_runtime->start;
    BaraniumHandle* next = NULL;
    BaraniumHandle* prev = NULL;
    while (currentHandle != handle && currentHandle != NULL)
    {
        next = currentHandle->next;
        currentHandle = next;
    }

    if (currentHandle == NULL)
    {
        LOGWARNING("Handle not found in runtime, is the given pointer right?");
        return;
    }

    if (currentHandle == current_active_runtime->start && currentHandle == current_active_runtime->end)
    {
        current_active_runtime->start = current_active_runtime->end = NULL;
        goto destroy;
    }

    if (currentHandle == current_active_runtime->start)
    {
        next = current_active_runtime->start->next;
        if (next)
            next->prev = NULL;
        current_active_runtime->start = next;
        goto destroy;
    }

    if (currentHandle == current_active_runtime->end)
    {
        prev = current_active_runtime->end->prev;
        if (prev)
            prev->next = NULL;
        current_active_runtime->end = prev;
        goto destroy;
    }

    prev = currentHandle->prev;
    next = currentHandle->next;
    prev->next = next;
    next->prev = prev;

destroy:
    if (handle->file)
        fclose(handle->file);

    free(handle);

    current_active_runtime->openHandles--;
}