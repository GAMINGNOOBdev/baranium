#ifndef __BARANIUM__RUNTIME_H_
#define __BARANIUM__RUNTIME_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"

/**
 * @brief Initialize a runtime instance
 * 
 * @returns An instance of the runtime and it's data
 */
BARANIUMAPI BaraniumRuntime* baranium_init();

/**
 * @brief Set the current global runtime context
 * 
 * @param runtimeContext The new runtime context
 */
BARANIUMAPI void baranium_set_context(BaraniumRuntime* runtimeContext);

/**
 * @brief Get the current global runtime context
 */
BARANIUMAPI BaraniumRuntime* baranium_get_context();

/**
 * @brief Safely dispose a runtime instance
 * 
 * @param runtime The instance of the runtime that should be disposed
 * 
 * @note `runtime` cannot be set to `null` to automatically dispose the global context
 */
BARANIUMAPI void baranium_cleanup(BaraniumRuntime* runtime);

/**
 * @brief Open a file handle
 * 
 * @param source Path to the file
 * 
 * @note This function will not check if the opened file is a baranium script
 * 
 * @returns A handle to the file
 */
BARANIUMAPI BaraniumHandle* baranium_open_handle(const char* source);

/**
 * @brief Close an open file handle
 * 
 * @param handle The handle that will be closed
 */
BARANIUMAPI void baranium_close_handle(BaraniumHandle* handle);

#ifdef __cplusplus
}
#endif

#endif