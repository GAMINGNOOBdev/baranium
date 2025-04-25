#ifndef __BARANIUM__COMPILER__BINARIES__COMPILER_H_
#define __BARANIUM__COMPILER__BINARIES__COMPILER_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/compiler/binaries/symbol_table.h>
#include <baranium/compiler/language/token.h>
#include <baranium/variable.h>
#include <baranium/script.h>
#include <stdint.h>

#define BARANIUM_CMP_LESS_THAN     0x18
#define BARANIUM_CMP_LESS_EQUAL    0x38
#define BARANIUM_CMP_GREATER_THAN  0x28
#define BARANIUM_CMP_GREATER_EQUAL 0x48
#define BARANIUM_CMP_EQUAL         0x08
#define BARANIUM_CMP_NOTEQUAL      0xF8

#define BARANIUM_CMP_AND 0
#define BARANIUM_CMP_OR  1

#define BARANIUM_CODE_BUFFER_SIZE   0x400

/**
* @brief A class that compiles tokens into executable binary code
*/
typedef struct
{
    uint8_t* code;
    size_t code_length;
    size_t code_buffer_size;
    baranium_symbol_table var_table;
    baranium_script_section* sections;
    size_t section_buffer_size;
    size_t section_count;

    size_t dependency_count;
    const char** dependencies;
} baranium_compiler;

/**
 * @brief Construct a new `baranium_compiler`
 * 
 * @param compiler The compiler that will be initialized
 */
BARANIUMAPI void baranium_compiler_init(baranium_compiler* compiler);

/**
 * @brief Dispose a new `baranium_compiler`
 * 
 * @param compiler The compiler that will be disposed
 */
BARANIUMAPI void baranium_compiler_dispose(baranium_compiler* compiler);

/**
 * @brief Write a final binary based off of the raw tokens from the **root** of the source
 * 
 * @param tokens The code tokens
 */
BARANIUMAPI void baranium_compiler_write(baranium_compiler* compiler, baranium_token_list* tokens, FILE* file, uint8_t library);

#ifdef __cplusplus
}
#endif

#endif
