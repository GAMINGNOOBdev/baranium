#ifndef __BARANIUM__COMPILER__PREPROCESSOR_H_
#define __BARANIUM__COMPILER__PREPROCESSOR_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/compiler/source_token.h>

/**
 * @brief Initialize the preprocessor
*/
BARANIUMAPI void baranium_preprocessor_init(void);

/**
 * @brief Dispose the preprocessor
*/
BARANIUMAPI void baranium_preprocessor_dispose(void);

/**
 * @brief Parse and act upon an operation
 * 
 * @note `operation` should NOT start with a '+'
 * 
 * @param operation The line that describes an operation
 * @param source The source from which the operation came
*/
BARANIUMAPI void baranium_preprocessor_parse(const char* operation, baranium_source_token_list* source);

/**
 * @brief Add an include path to search for files
 * 
 * @note there should be something like a config to add/remove include paths, including relative paths
 * 
 * @param path The new include path
*/
BARANIUMAPI void baranium_preprocessor_add_include_path(const char* path);

/**
 * @brief Remove the last added include path
*/
BARANIUMAPI void baranium_preprocessor_pop_last_include(void);

/**
 * @brief Add a define
 * 
 * @param define Define name
 * @param replacement What the define replaces
*/
BARANIUMAPI void baranium_preprocessor_add_define(const char* define, const char* replacement);

/**
 * @brief Check if there are any defines in the currently available tokens and replace if necessary
 * 
 * @note This function will modify the given input parameter
 * 
 * @param line_tokens Line tokens
*/
BARANIUMAPI void baranium_preprocessor_assist_in_line(baranium_source_token_list* line_tokens);

/**
 * @brief Search for a file inside the currently given include paths
 * 
 * @param file The file that is required
 * 
 * @returns The full path to the file, empty string if not found
*/
BARANIUMAPI const char* baranium_preprocessor_search_include_path(const char* file);

#ifdef __cplusplus
}
#endif

#endif
