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
BARANIUMAPI void baranium_compiler_write(baranium_compiler* compiler, baranium_token_list* tokens, FILE* file);

/**
 * @brief Compile a list of tokens (does not write to a file!)
 * 
 * @param tokens The code tokens
 */
void baranium_compiler_compile(baranium_compiler* compiler, baranium_token_list* tokens);

/**
 * @brief Initialize the `baranium_compiler_code_builder` object
 */
void baranium_compiler_code_builder_init(baranium_compiler* compiler);

/**
 * @brief Dispose the `baranium_compiler_code_builder` object
 */
void baranium_compiler_code_builder_dispose(baranium_compiler* compiler);

/**
 * @brief Clear the currently held code
 */
void baranium_compiler_code_builder_clear(baranium_compiler* compiler);

/**
 * @brief Check if the last instruction is a return statement
 */
uint8_t baranium_compiler_code_builder_returned_from_execution(baranium_compiler* compiler);

// push a string value to the stack (needed for assignments of strings)
void baranium_compiler_code_builder_push_string(baranium_compiler* compiler, const char* str);

// push a bool value to the stack
void baranium_compiler_code_builder_push_bool(baranium_compiler* compiler, uint8_t b);

// push a uint value to the stack
void baranium_compiler_code_builder_push_uint(baranium_compiler* compiler, uint32_t val);

// push a int value to the stack
void baranium_compiler_code_builder_push_int(baranium_compiler* compiler, int32_t val);

// push a float value to the stack
void baranium_compiler_code_builder_push_float(baranium_compiler* compiler, float val);

void baranium_compiler_code_builder_push64(baranium_compiler* compiler, uint64_t data);
void baranium_compiler_code_builder_push32(baranium_compiler* compiler, uint32_t data);
void baranium_compiler_code_builder_push16(baranium_compiler* compiler, uint16_t data);
void baranium_compiler_code_builder_push(baranium_compiler* compiler, uint8_t data);

//////////////////////
///                ///
///   OPERATIONS   ///
///                ///
//////////////////////

// no operation
void baranium_compiler_code_builder_NOP(baranium_compiler* compiler);

// clear compare flag
void baranium_compiler_code_builder_CCF(baranium_compiler* compiler);

// set compare flag
void baranium_compiler_code_builder_SCF(baranium_compiler* compiler);

// clear compare value
void baranium_compiler_code_builder_CCV(baranium_compiler* compiler);

// invert compare value
void baranium_compiler_code_builder_ICV(baranium_compiler* compiler);

// push the compare value to the stack
void baranium_compiler_code_builder_PUSHCV(baranium_compiler* compiler);

// pop the compare value from the stack
void baranium_compiler_code_builder_POPCV(baranium_compiler* compiler);

// push a variable's value to the stack (can't be used on string variables fyi)
void baranium_compiler_code_builder_PUSHVAR(baranium_compiler* compiler, index_t id);

// pop a value from the stack into a variable
void baranium_compiler_code_builder_POPVAR(baranium_compiler* compiler, index_t id);

// push a value to the stack
void baranium_compiler_code_builder_PUSH(baranium_compiler* compiler, uint64_t val);

// call a function with a specific id
void baranium_compiler_code_builder_CALL(baranium_compiler* compiler, index_t id);

// return from a function
void baranium_compiler_code_builder_RET(baranium_compiler* compiler);

// jump to
void baranium_compiler_code_builder_JMP(baranium_compiler* compiler, uint64_t addr);

// jump offset-ed from the current position to
void baranium_compiler_code_builder_JMPOFF(baranium_compiler* compiler, int16_t offset);

// jump if equal to
void baranium_compiler_code_builder_JMPC(baranium_compiler* compiler, uint64_t addr);

// jump offset-ed from the current position
void baranium_compiler_code_builder_JMPCOFF(baranium_compiler* compiler, int16_t addr);

// modulo two values from the stack
void baranium_compiler_code_builder_MOD(baranium_compiler* compiler);

// divide two values from the stack
void baranium_compiler_code_builder_DIV(baranium_compiler* compiler);

// multiply two values from the stack
void baranium_compiler_code_builder_MUL(baranium_compiler* compiler);

// subtrack two values from the stack
void baranium_compiler_code_builder_SUB(baranium_compiler* compiler);

// add two values from the stack
void baranium_compiler_code_builder_ADD(baranium_compiler* compiler);

// bitwise and two values from the stack
void baranium_compiler_code_builder_AND(baranium_compiler* compiler);

// bitwise or two values from the stack
void baranium_compiler_code_builder_OR(baranium_compiler* compiler);

// bitwise exclusive-or two values from the stack
void baranium_compiler_code_builder_XOR(baranium_compiler* compiler);

// bitwise shift left
void baranium_compiler_code_builder_SHFTL(baranium_compiler* compiler);

// bitwise shift right
void baranium_compiler_code_builder_SHFTR(baranium_compiler* compiler);

// compare two values on the stack
void baranium_compiler_code_builder_CMP(baranium_compiler* compiler, uint8_t compareMethod);

// compare two compared values together
void baranium_compiler_code_builder_CMPC(baranium_compiler* compiler, uint8_t compareCombineMethod);

// allocate memory
void baranium_compiler_code_builder_MEM(baranium_compiler* compiler, size_t size, uint8_t type, index_t id);

// deallocate memory
void baranium_compiler_code_builder_FEM(baranium_compiler* compiler, index_t id);

// set var data
void baranium_compiler_code_builder_SET(baranium_compiler* compiler, index_t id, size_t size, baranium_value_t data, baranium_variable_type_t type);

// instantiate an object
void baranium_compiler_code_builder_INSTANTIATE(baranium_compiler* compiler);

// delete an object
void baranium_compiler_code_builder_DELETE(baranium_compiler* compiler);

// attach to an object
void baranium_compiler_code_builder_ATTACH(baranium_compiler* compiler);

// detach from an object
void baranium_compiler_code_builder_DETACH(baranium_compiler* compiler);

// stop execution with return code
void baranium_compiler_code_builder_KILL(baranium_compiler* compiler, int64_t code);

#ifdef __cplusplus
}
#endif

#endif
