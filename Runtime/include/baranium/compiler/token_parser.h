#ifndef __BARANIUM__COMPILER__TOKEN_PARSER_H_
#define __BARANIUM__COMPILER__TOKEN_PARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/compiler/language/function_token.h>
#include <baranium/compiler/language/token.h>
#include <baranium/compiler/source_token.h>
#include <baranium/variable.h>

/**
 * @brief An object that parses tokens from a source to then create a final binary
 */
typedef struct
{
    baranium_token_list tokens;
} baranium_token_parser;

/**
 * @brief Construct a new token parser
 */
BARANIUMAPI void baranium_token_parser_init(baranium_token_parser* parser);

/**
 * @brief Parse the incoming tokens
 * 
 * @param tokens Tokens that will be parsed
 */
BARANIUMAPI void baranium_token_parser_parse(baranium_token_parser* parser, baranium_source_token_list* tokens);

/**
 * @brief Dispose a token parser
 */
BARANIUMAPI void baranium_token_parser_dispose(baranium_token_parser* parser);

/**
 * @brief Read a variable
 * 
 * @param index Index of the current token
 * @param current The current token
 * @param tokens List of all tokens including the current one
 * @param output The output list where the read token will be saved
 * @param global_tokens A list where the tokens that are globally available are saved
 */
BARANIUMAPI void baranium_token_parser_parse_variable(size_t* index, baranium_source_token* current, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens);

/**
 * @brief Read a field
 * 
 * @param index Index of the current token
 * @param current The current token
 * @param tokens List of all tokens including the current one
 * @param output The output list where the read token will be saved
 * @param global_tokens A list where the tokens that are globally available are saved
 */
BARANIUMAPI void baranium_token_parser_parse_field(size_t* index, baranium_source_token* current, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens);

/**
 * @brief Read an expression
 * 
 * @param index Index of the current token
 * @param current The current token
 * @param tokens List of all tokens including the current one
 * @param output The output list where the read token will be saved
 * @param global_tokens A list where the tokens that are globally available are saved
 * 
 * @returns True if this expression is a return statement
 */
BARANIUMAPI uint8_t baranium_token_parser_parse_expression(size_t* index, baranium_source_token* current, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens);

/**
 * @brief Read a function
 * 
 * @param index Index of the current token
 * @param current The current token
 * @param tokens List of all tokens including the current one
 * @param output The output list where the read token will be saved
 * @param global_tokens A list where the tokens that are globally available are saved
 */
BARANIUMAPI void baranium_token_parser_parse_function(size_t* index, baranium_source_token* current, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens);

/**
 * @brief Read an if-else-statement
 * 
 * @param index Index of the current token
 * @param current The current token
 * @param tokens List of all tokens including the current one
 * @param output The output list where the read token will be saved
 * @param global_tokens A list where the tokens that are globally available are saved
 */
BARANIUMAPI void baranium_token_parser_parse_if_statement(size_t* index, baranium_source_token* current, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens);

/**
 * @brief Read a loop
 * 
 * @param index Index of the current token
 * @param current The current token
 * @param tokens List of all tokens including the current one
 * @param output The output list where the read token will be saved
 * @param global_tokens A list where the tokens that are globally available are saved
 */
BARANIUMAPI void baranium_token_parser_parse_loop(size_t* index, baranium_source_token* current, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens);

/**
 * @brief Read a function parameter
 * 
 * @param index Index of the current token
 * @param function Parent function
 * @param current The current token
 * @param tokens List of all tokens including the current one
 */
BARANIUMAPI void baranium_token_parser_parse_function_parameter(size_t* index, baranium_function_token* function, baranium_source_token* current, baranium_source_token_list* tokens);

/**
 * @brief Parse tokens and return the value of a variable depending on it's type
 * 
 * @param tokens List of tokens that will be parsed
 * @param varType The type of the variable
 * 
 * @return The value
 */
BARANIUMAPI const char* baranium_token_parser_parse_variable_value(baranium_source_token_list* tokens, baranium_variable_type_t varType);

/**
 * @brief Read contents (i.e. if/else statements and loops, expressions) that
 *        start with a specific type of token and have a corresponding ending token
 * 
 * @note Will have to be called once it is identified that there are contents,
 *       meaning the next token will NOT be of type `startType`. The current
 *       `index` will be at the last instance of a token with the type `endType`,
 *       meaning to proceed with reading other tokens, first increase the
 *       value of `index`.
 * 
 * 
 * @param index Index of the current token
 * @param startType Token type of the start
 * @param endType Token type of the end
 * @param tokens List of all tokens including the start and end one
 * @param output The output list of all tokens that are considered as content
 * 
 * @returns `true` when the depth is the same as when it was in the beginning,
 *          otherwise `false`
 */
BARANIUMAPI uint8_t baranium_token_parser_parse_content_using_depth(size_t* index, baranium_source_token_type_t startType, baranium_source_token_type_t endType, baranium_source_token_list* tokens, baranium_source_token_list* output);

#ifdef __cplusplus
}
#endif

#endif
