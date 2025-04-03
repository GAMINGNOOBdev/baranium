#ifndef __BARANIUM__COMPILER__LANGUAGE__EXPRESSION_H_
#define __BARANIUM__COMPILER__LANGUAGE__EXPRESSION_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/compiler/language/abstract_syntax_tree.h>
#include <baranium/compiler/language/variable_token.h>
#include <baranium/compiler/language/language.h>
#include <baranium/compiler/language/token.h>
#include <baranium/compiler/source_token.h>
#include <baranium/variable.h>
#include <stdint.h>

#define BARANIUM_EXPRESSION_TYPE_INVALID                (baranium_expression_type_t)-1
#define BARANIUM_EXPRESSION_TYPE_NONE                   (baranium_expression_type_t)0
#define BARANIUM_EXPRESSION_TYPE_CONDITION              (baranium_expression_type_t)1
#define BARANIUM_EXPRESSION_TYPE_ASSIGNMENT             (baranium_expression_type_t)2
#define BARANIUM_EXPRESSION_TYPE_FUNCTION_CALL          (baranium_expression_type_t)3
#define BARANIUM_EXPRESSION_TYPE_RETURN_STATEMENT       (baranium_expression_type_t)4
#define BARANIUM_EXPRESSION_TYPE_KEYWORD_EXPRESSION     (baranium_expression_type_t)5
#define BARANIUM_EXPRESSION_TYPE_ARITHMETIC_OPERATION   (baranium_expression_type_t)6

typedef int baranium_expression_type_t;

/**
 * @brief Get the string representation of an expression type
 *
 * @param type The expression type that will be "stringified"
 *
 * @return The string representation of `type`
 */
BARANIUMAPI const char* baranium_expression_type_to_string(baranium_expression_type_t type);

typedef struct baranium_expression_token
{
    baranium_token base;
    int line_number;
    baranium_expression_type_t expression_type;
    baranium_abstract_syntax_tree_node* ast;
    const char* return_value;
    baranium_variable_type_t return_type;
    baranium_source_token_list inner_tokens;
    const char* return_variable;
    struct baranium_expression_token* return_expression;
} baranium_expression_token;

/**
* @brief Construct a new expression object
*/
BARANIUMAPI void baranium_expression_token_init(baranium_expression_token* expression);

/**
* @brief Dispose a new expression object
*/
BARANIUMAPI void baranium_expression_token_dispose(baranium_expression_token* expression);

/**
* @brief Identify the type of this expression
*/
BARANIUMAPI void baranium_expression_token_identify(baranium_expression_token* expression, baranium_token_list* local_tokens, baranium_token_list* global_tokens);

#ifdef __cplusplus
}
#endif

#endif
