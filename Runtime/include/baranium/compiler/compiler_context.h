#ifndef __BARANIUM__COMPILER__CONTEXT_H_
#define __BARANIUM__COMPILER__CONTEXT_H_ 1

#ifdef __cplusplus
extern "C" {
#endif
    
#include <baranium/compiler/token_parser.h>
#include <baranium/compiler/source_token.h>
#include <baranium/string_util.h>
#include <baranium/library.h>
#include <baranium/defines.h>

#define BARANIUM_COMPILER_CONTEXT_LIBRARY_BUFFER_SIZE 0x20

typedef struct baranium_compiler_context
{
    baranium_source_token_list combined_source;
    baranium_token_parser token_parser;
    baranium_string_map nametable;
    uint64_t library_buffer_size;
    baranium_library** libraries;
    uint64_t library_count;

    baranium_string_list library_dir_contents;
    char* library_dir_path;
} baranium_compiler_context;

/**
 * @brief Initialize a compiler context
 * 
 * @returns An instance of the compiler context
 */
BARANIUMAPI baranium_compiler_context* baranium_compiler_context_init(void);

/**
 * @brief Set the current global compiler context
 * 
 * @param ctx The new compiler context
 */
BARANIUMAPI void baranium_set_compiler_context(baranium_compiler_context* ctx);

/**
 * @brief Get the current global compiler context
 */
BARANIUMAPI baranium_compiler_context* baranium_get_compiler_context(void);

/**
 * @brief Safely dispose a compiler context
 * 
 * @param compiler The instance of the compiler context that should be disposed
 * 
 * @note `compiler` cannot be set to `null` to automatically dispose the global context
 */
BARANIUMAPI void baranium_compiler_context_dispose(baranium_compiler_context* ctx);

/**
 * @brief Set the current library directory of this compiler context
 * 
 * @param ctx Compiler context
 * @param str Library directory path
 */
BARANIUMAPI void baranium_compiler_context_set_library_directory(baranium_compiler_context* ctx, const char* str);

/**
 * @brief Add a source file to the current compilation list
 * 
 * @param ctx Compiler context
 * @param source Source file
 */
BARANIUMAPI void baranium_compiler_context_add_source(baranium_compiler_context* ctx, FILE* source);

/**
 * @brief Clear all source files from the current compilation list
 * 
 * @param ctx Compiler context
 */
BARANIUMAPI void baranium_compiler_context_clear_sources(baranium_compiler_context* ctx);

/**
 * @brief Lookup a symbolname inside the currently loaded libraries
 * 
 * @param ctx Compiler context
 * @param symbolname Symbol name
 * 
 * @returns A pointer to the library that actually holds the symbol
 */
BARANIUMAPI baranium_library* baranium_compiler_context_lookup(baranium_compiler_context* ctx, const char* symbolname);

/**
 * @brief Compile the resulting binary
 * 
 * @param ctx Compiler context
 * @param output Output file path
 * @param library Whether the output binary should be a library or not
 */
BARANIUMAPI void baranium_compiler_context_compile(baranium_compiler_context* ctx, const char* output, uint8_t library);

/**
 * @brief Add a library to a compiler context
 * 
 * @param ctx The compiler context
 * @param name The library name
 */
BARANIUMAPI void baranium_compiler_context_add_library(baranium_compiler_context* ctx, const char* name);

#ifdef __cplusplus
}
#endif

#endif
