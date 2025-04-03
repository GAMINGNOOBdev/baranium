#ifndef __BARANIUM__COMPILER__LANGUAGE__IF_ELSE_TOKEN_H_
#define __BARANIUM__COMPILER__LANGUAGE__IF_ELSE_TOKEN_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/compiler/language/expression_token.h>
#include <baranium/compiler/language/language.h>
#include <baranium/compiler/language/token.h>
#include <baranium/compiler/source_token.h>
#include <stdint.h>

typedef struct baranium_if_else_token
{
    baranium_token base;
    baranium_expression_token condition;
    baranium_source_token_list inner_tokens;
    baranium_token_list tokens;
    baranium_token_list chained_statements;
    uint8_t has_else_statement : 1;
    uint8_t reserved : 7;
} baranium_if_else_token;

BARANIUMAPI void baranium_if_else_token_init(baranium_if_else_token* ifelsetoken);
BARANIUMAPI void baranium_if_else_token_dispose(baranium_if_else_token* ifelsetoken);

/**
 * @brief Identify the type of this expression
 */
BARANIUMAPI void baranium_if_else_token_parse(baranium_if_else_token* ifelsetoken, baranium_token_list* local_tokens, baranium_token_list* global_tokens);

#ifdef __cplusplus
}
#endif

#endif
