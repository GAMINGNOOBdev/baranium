#ifndef __BARANIUM__RUNTIME_H_
#define __BARANIUM__RUNTIME_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/defines.h>

/**
 * @brief Initialize a runtime instance
 * 
 * @returns An instance of the runtime and it's data
 */
BARANIUMAPI baranium_runtime* baranium_init(void);

/**
 * @brief Set the current global runtime context
 * 
 * @param runtimeContext The new runtime context
 */
BARANIUMAPI void baranium_set_context(baranium_runtime* runtimeContext);

/**
 * @brief Get the current global runtime context
 */
BARANIUMAPI baranium_runtime* baranium_get_context(void);

/**
 * @brief Safely dispose a runtime instance
 * 
 * @param runtime The instance of the runtime that should be disposed
 * 
 * @note `runtime` cannot be set to `null` to automatically dispose the global context
 */
BARANIUMAPI void baranium_cleanup(baranium_runtime* runtime);

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
