#ifndef __BARANIUM__COMPILER__LANGUAGE__FUNCTION_H_
#define __BARANIUM__COMPILER__LANGUAGE__FUNCTION_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/compiler/language/variable_token.h>
#include <baranium/compiler/language/token.h>
#include <baranium/compiler/source_token.h>
#include <baranium/variable.h>

typedef struct
{
    baranium_token base;
    baranium_variable_type_t return_type;
    const char* return_value;
    const char* return_variable;
    uint8_t only_declaration : 1;
    uint8_t reserved : 7;

    baranium_token_list parameters; // should only contain baranium_variable_token type objects
    baranium_source_token_list inner_tokens;
    baranium_token_list tokens;
} baranium_function_token;

BARANIUMAPI void baranium_function_token_init(baranium_function_token* function);
BARANIUMAPI void baranium_function_token_dispose(baranium_function_token* function);
BARANIUMAPI void baranium_function_token_parse(baranium_function_token* function, baranium_token_list* global_tokens);

#ifdef __cplusplus
}
#endif

#endif
