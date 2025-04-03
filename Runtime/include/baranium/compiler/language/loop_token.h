#ifndef __BARANIUM__COMPILER__LANGUAGE__LOOP_TOKEN_H_
#define __BARANIUM__COMPILER__LANGUAGE__LOOP_TOKEN_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/compiler/language/expression_token.h>
#include <baranium/compiler/language/variable_token.h>
#include <baranium/compiler/language/token.h>
#include <baranium/compiler/source_token.h>

typedef struct
{
    baranium_token base;
    uint8_t t_do_while : 1;
    uint8_t t_while : 1;
    uint8_t t_reserved;
    baranium_variable_token start_variable;
    baranium_expression_token start_expression;
    baranium_expression_token condition;
    baranium_expression_token iteration;
    baranium_source_token_list inner_tokens;
    baranium_token_list tokens;
} baranium_loop_token;

/**
 * @brief Construct a new loop token
 */
BARANIUMAPI void baranium_loop_token_init(baranium_loop_token* loop);

/**
 * @brief Dispose a loop token
 */
BARANIUMAPI void baranium_loop_token_dispose(baranium_loop_token* loop);

/**
* @brief Identify and parse inner tokens
*/
BARANIUMAPI void baranium_loop_token_parse(baranium_loop_token* loop, baranium_token_list* local_tokens, baranium_token_list* global_tokens);

/**
* @brief Set the type of a loop by identifying it from a source token
* 
* @param loop The loop token that will get the identified type
* @param loop_identifier The token that will be used to identify the loop type
*/
BARANIUMAPI void baranium_loop_token_type_from_source_token(baranium_loop_token* loop, baranium_source_token* loop_identifier);

#ifdef __cplusplus
}
#endif

#endif
