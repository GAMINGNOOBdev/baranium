#include <baranium/backend/bfuncmgr.h>
#include <baranium/cpu/bcpu_opcodes.h>
#include <baranium/backend/bvarmgr.h>
#include <baranium/cpu/bstack.h>
#include <baranium/callback.h>
#include <baranium/runtime.h>
#include <baranium/defines.h>
#include <baranium/logging.h>
#include <baranium/bcpu.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static baranium_runtime* current_active_runtime = NULL;

baranium_runtime* baranium_init(void)
{
    bcpu_opcodes_init();

    baranium_runtime* runtimeHandle = malloc(sizeof(baranium_runtime));
    if (!runtimeHandle)
        return NULL;

    memset(runtimeHandle, 0, sizeof(baranium_runtime));

    runtimeHandle->cpu = bcpu_init(runtimeHandle);
    runtimeHandle->functionStack = bstack_init();
    runtimeHandle->functionManager = baranium_function_manager_init();
    runtimeHandle->callbacks = baranium_callback_list_init();
    runtimeHandle->varmgr = bvarmgr_init();
    return runtimeHandle;
}

void baranium_set_context(baranium_runtime* runtimeContext)
{
    current_active_runtime = runtimeContext;
}

baranium_runtime* baranium_get_context(void)
{
    return current_active_runtime;
}

void baranium_cleanup(baranium_runtime* runtime)
{
    if (runtime == NULL) return;

    if (!(runtime->start == NULL && runtime->end == NULL))
    {
        baranium_handle* handle = runtime->start;
        baranium_handle* next = handle->next;
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
    baranium_callback_list_dispose(runtime->callbacks);
    baranium_function_manager_dispose(runtime->functionManager);
    bvarmgr_dispose(runtime->varmgr);
    free(runtime);
}

index_t baranium_get_id_of_name(const char* name)
{
    uint64_t identifier = 5381;
    size_t length = strlen(name);
    for (size_t i = 0; i < length; i++)
    {
        char c = name[i];
        identifier = ((identifier << 5) + identifier) + c;
    }
    return (index_t)identifier;
}

baranium_handle* baranium_open_handle(const char* source)
{
    if (current_active_runtime == NULL)
        return NULL;

    FILE* file = fopen(source, "rb");
    if (file == NULL)
    {
        LOGERROR(stringf("File '%s' not found", source));
        return NULL;
    }

    baranium_handle* handle = (baranium_handle*)malloc(sizeof(baranium_handle));
    if (!handle)
        return NULL;

    memset(handle, 0, sizeof(baranium_handle));
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

void baranium_close_handle(baranium_handle* handle)
{
    if (current_active_runtime == NULL || handle == NULL)
        return;

    baranium_handle* currentHandle = current_active_runtime->start;
    baranium_handle* next = NULL;
    baranium_handle* prev = NULL;
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
