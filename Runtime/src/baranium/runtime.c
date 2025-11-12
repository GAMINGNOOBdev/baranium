#include <baranium/backend/bfuncmgr.h>
#include <baranium/cpu/bcpu_opcodes.h>
#include <baranium/backend/bvarmgr.h>
#include <baranium/string_util.h>
#include <baranium/cpu/bstack.h>
#include <baranium/file_util.h>
#include <baranium/callback.h>
#include <baranium/runtime.h>
#include <baranium/defines.h>
#include <baranium/logging.h>
#include <baranium/library.h>
#include <baranium/bcpu.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static baranium_runtime* current_active_runtime = NULL;

baranium_runtime* baranium_init_runtime(void)
{
    bcpu_opcodes_init();

    baranium_runtime* runtimeHandle = malloc(sizeof(baranium_runtime));
    if (!runtimeHandle)
        return NULL;

    memset(runtimeHandle, 0, sizeof(baranium_runtime));

    runtimeHandle->cpu = bcpu_init(runtimeHandle);
    runtimeHandle->function_stack = bstack_init();
    runtimeHandle->function_manager = baranium_function_manager_init();
    runtimeHandle->callbacks = baranium_callback_list_init();
    runtimeHandle->varmgr = bvarmgr_init();
    return runtimeHandle;
}

void baranium_set_runtime(baranium_runtime* runtimeContext)
{
    current_active_runtime = runtimeContext;
}

baranium_runtime* baranium_get_runtime(void)
{
    return current_active_runtime;
}

void baranium_runtime_set_library_path(const char* library_path)
{
    if (current_active_runtime == NULL || library_path == NULL)
        return;

    if (current_active_runtime->library_dir_contents == NULL)
        current_active_runtime->library_dir_contents = malloc(sizeof(baranium_string_list));

    if (current_active_runtime->library_path != NULL)
    {
        free((void*)current_active_runtime->library_path);
            baranium_string_list_dispose(current_active_runtime->library_dir_contents);
    }

    size_t len = strlen(library_path);
    if (library_path[len-1] == '/' || library_path[len-1] == '\\')
        len--;

    current_active_runtime->library_path = malloc(len+1);
    strncpy((char*)current_active_runtime->library_path, library_path, len);
    ((char*)current_active_runtime->library_path)[len] = 0;
    *current_active_runtime->library_dir_contents = baranium_file_util_get_directory_contents(current_active_runtime->library_path, BARANIUM_FILE_UTIL_FILTER_MASK_ALL_FILES);
}

void baranium_runtime_load_dependency(const char* dependency)
{
    if (current_active_runtime == NULL)
        return;

    LOGDEBUG("loading dependency '%s' from '%s'", dependency, current_active_runtime->library_path);
    baranium_library* library = NULL;

    for (size_t i = 0; i < current_active_runtime->library_dir_contents->count; i++)
    {
        const char* path = current_active_runtime->library_dir_contents->strings[i];
        char* filenameptr = (char*)path;
        for (int i = strlen(path)-1; i > 0; i--)
        {
            if (path[i] == '\\' || path[i] == '/')
            {
                filenameptr = (char*)&path[i+1];
                break;
            }
        }
        if (strcmp(filenameptr, dependency) == 0)
        {
            library = baranium_library_load(stringf("%s/%s", current_active_runtime->library_path, path));
            break;
        }
    }

    if (library == NULL)
        return;

    if (current_active_runtime->library_count + 1 >= current_active_runtime->library_buffer_size)
    {
        current_active_runtime->library_buffer_size += BARANIUM_RUNTIME_LIBRARY_BUFFER_SIZE;
        current_active_runtime->libraries = realloc(current_active_runtime->libraries, sizeof(baranium_library*)*current_active_runtime->library_buffer_size);
    }

    current_active_runtime->libraries[current_active_runtime->library_count] = library;
    current_active_runtime->library_count++;
}

void baranium_dispose_runtime(baranium_runtime* runtime)
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

    for (size_t i = 0; i < runtime->library_count; i++)
        baranium_library_dispose(runtime->libraries[i]);

    if (runtime->libraries)
        free(runtime->libraries);

    if (runtime->library_path)
        free((void*)runtime->library_path);

    if (runtime->library_dir_contents)
    {
        baranium_string_list_dispose(runtime->library_dir_contents);
        free(runtime->library_dir_contents);
    }

    bcpu_dispose(runtime->cpu);
    bstack_dispose(runtime->function_stack);
    baranium_callback_list_dispose(runtime->callbacks);
    baranium_function_manager_dispose(runtime->function_manager);
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
        LOGERROR("File '%s' not found", source);
        return NULL;
    }

    baranium_handle* handle = (baranium_handle*)malloc(sizeof(baranium_handle));
    if (!handle)
        return NULL;

    memset(handle, 0, sizeof(baranium_handle));
    handle->file = file;
    handle->path = strdup(source);
    if (!handle->path)
    {
        LOGERROR("Couldn't create handle, insufficient memory");
        free(handle);
        return NULL;
    }

    if (current_active_runtime->start == NULL)
    {
        current_active_runtime->start = current_active_runtime->end = handle;
        current_active_runtime->open_handles = 1;
        goto end;
    }

    if (current_active_runtime->start == current_active_runtime->end)
    {
        handle->prev = current_active_runtime->start;
        current_active_runtime->start->next = current_active_runtime->end = handle;
        current_active_runtime->open_handles++;
        goto end;
    }

    handle->prev = current_active_runtime->end;
    current_active_runtime->end->next = handle;
    current_active_runtime->end = handle;
    current_active_runtime->open_handles++;

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

    if (handle->path)
        free(handle->path);

    free(handle);

    current_active_runtime->open_handles--;
}
