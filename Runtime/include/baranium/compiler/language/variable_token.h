#ifndef __BARANIUM__COMPILER__LANGUAGE__VARIABLE_TOKEN_H_
#define __BARANIUM__COMPILER__LANGUAGE__VARIABLE_TOKEN_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/compiler/language/token.h>
#include <baranium/compiler/source_token.h>
#include <baranium/variable.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief Get the variable type from a single token
 * 
 * @param token The token that will be converted into the variable type
 * @returns The variable type
*/
BARANIUMAPI baranium_variable_type_t baranium_variable_type_from_token(baranium_source_token* token);

/**
 * @brief Predict the variable type from a list of tokens
 * 
 * @param tokens The list of tokens that represent the variable type
 * @returns The predicted variable type 
*/
BARANIUMAPI baranium_variable_type_t baranium_variable_predict_type(baranium_source_token_list* tokens);

typedef struct
{
    baranium_token base;
    baranium_variable_type_t type;
    size_t array_size;
    const char* value;
} baranium_variable_token;

BARANIUMAPI void baranium_variable_token_init(baranium_variable_token* variable);
BARANIUMAPI void baranium_variable_token_dispose(baranium_variable_token* variable);

#ifdef __cplusplus
}
#endif

#endif
