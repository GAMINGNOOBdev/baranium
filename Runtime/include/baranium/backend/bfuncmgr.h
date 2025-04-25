/**
 * @note THIS IS NOT INTENDED FOR USE BY THE USER OF THE RUNTIME!
 *       This header is intended to be used internally by the runtime
 *       and therefore, functions defined in this header cannot be used
 *       by the user.
 */
#ifndef __BARANIUM__BACKEND__BFUNCMGR_H_
#define __BARANIUM__BACKEND__BFUNCMGR_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/function.h>
#include <baranium/library.h>
#include <baranium/script.h>
#include <stdbool.h>

#define BARANIUM_FUNCTION_MANAGER_BUFFER_SIZE 0x20

typedef struct
{
    index_t id;
    baranium_script* script;
    baranium_library* library;
} baranium_function_manager_entry;

typedef struct baranium_function_manager
{
    baranium_function_manager_entry* buffer;
    size_t buffer_size;
    size_t count;
} baranium_function_manager;

// create and initialize a function manager
baranium_function_manager* baranium_function_manager_init(void);

// dispose a function manager
void baranium_function_manager_dispose(baranium_function_manager* obj);

// clear a function manager
void baranium_function_manager_clear(baranium_function_manager* obj);

// allocate/create a function entry
void baranium_function_manager_add(baranium_function_manager* obj, index_t id, baranium_script* script, baranium_library* library);

// get a function if existent
baranium_function* baranium_function_manager_get(baranium_function_manager* obj, index_t id);

// get a function entry if existent
baranium_function_manager_entry* baranium_function_manager_get_entry(baranium_function_manager* obj, index_t id);

// get a function entry index if existent
int baranium_function_manager_get_entry_index(baranium_function_manager* obj, index_t id);

// delete and free memory used by a function entry
void baranium_function_manager_remove(baranium_function_manager* obj, index_t id);

#ifdef __cplusplus
}
#endif

#endif
