#ifndef __BARANIUM__RUNTIME_H_
#define __BARANIUM__RUNTIME_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/defines.h>

#define BARANIUM_RUNTIME_LIBRARY_BUFFER_SIZE 0x20

/**
 * @brief Initialize a runtime instance
 * 
 * @returns An instance of the runtime and it's data
 */
BARANIUMAPI baranium_runtime* baranium_init_runtime(void);

/**
 * @brief Set the current global runtime
 * 
 * @param runtimeContext The new runtime
 */
BARANIUMAPI void baranium_set_runtime(baranium_runtime* runtimeContext);

/**
 * @brief Get the current global runtime
 */
BARANIUMAPI baranium_runtime* baranium_get_runtime(void);

/**
 * @brief Set the library path of the current runtime
 * 
 * @param library_path Library path
 */
 BARANIUMAPI void baranium_runtime_set_library_path(const char* library_path);

/**
 * @brief Load a dependency/library to the current runtime
 * 
 * @param dependency Dependency name
 */
BARANIUMAPI void baranium_runtime_load_dependency(const char* dependency);

/**
 * @brief Safely dispose a runtime instance
 * 
 * @param runtime The instance of the runtime that should be disposed
 * 
 * @note `runtime` cannot be set to `null` to automatically dispose the global
 */
BARANIUMAPI void baranium_dispose_runtime(baranium_runtime* runtime);

/**
 * @brief Get the ID of a named object or function
 *
 * @param name Name of the object
 */
BARANIUMAPI index_t baranium_get_id_of_name(const char* name);

/**
 * @brief Open a file handle
 * 
 * @param source Path to the file
 * 
 * @note This function will not check if the opened file is a baranium script
 * 
 * @returns A handle to the file
 */
BARANIUMAPI baranium_handle* baranium_open_handle(const char* source);

/**
 * @brief Close an open file handle
 * 
 * @param handle The handle that will be closed
 */
BARANIUMAPI void baranium_close_handle(baranium_handle* handle);

#ifdef __cplusplus
}
#endif

#endif
