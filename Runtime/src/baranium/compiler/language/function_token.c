#include <baranium/compiler/language/expression_token.h>
#include <baranium/compiler/language/function_token.h>
#include <baranium/compiler/language/language.h>
#include <baranium/compiler/language/token.h>
#include <baranium/compiler/source_token.h>
#include <baranium/compiler/token_parser.h>
#include <baranium/string_util.h>
#include <baranium/variable.h>
#include <baranium/runtime.h>
#include <baranium/logging.h>

uint8_t baranium_function_token_return_requested(baranium_function_token* function)
{
    if (function->tokens.count == 0)
        return 0;

    if (function->tokens.data[function->tokens.count-1]->type != BARANIUM_TOKEN_TYPE_EXPRESSION)
        return 0;

    baranium_expression_token* expression = (baranium_expression_token*)function->tokens.data[function->tokens.count-1];
    if (expression->expression_type == BARANIUM_EXPRESSION_TYPE_RETURN_STATEMENT)
    {
        function->return_type = expression->return_type;
        function->return_value = expression->return_value;
        function->return_variable = expression->return_variable;

        return 1;
    }

    return 0;
}

void baranium_function_token_init(baranium_function_token* function)
{
    function->base.id = BARANIUM_INVALID_INDEX;
    function->base.type = BARANIUM_TOKEN_TYPE_FUNCTION;
    function->return_type = BARANIUM_VARIABLE_TYPE_VOID;
    function->return_value = NULL;
    function->return_variable = NULL;
    function->only_declaration = 0;
    baranium_token_list_init(&function->tokens);
    baranium_token_list_init(&function->parameters);
    baranium_source_token_list_init(&function->inner_tokens);
}

void baranium_function_token_dispose(baranium_function_token* function)
{
    baranium_token_list_dispose(&function->tokens);
    baranium_token_list_dispose(&function->parameters);
    baranium_source_token_list_dispose(&function->inner_tokens, 0);
}

void baranium_function_token_parse(baranium_function_token* function, baranium_token_list* global_tokens)
{
    size_t index = 0;

    size_t parameters_index = function->tokens.count;
    for (size_t i = 0; i < function->parameters.count; i++)
        baranium_token_list_add(&function->tokens, function->parameters.data[i]);
    
    for (; index < function->inner_tokens.count && !baranium_function_token_return_requested(function); index++)
    {
        baranium_source_token* token = baranium_source_token_list_get(&function->inner_tokens, index);

        if (token->special_index == BARANIUM_KEYWORD_INDEX_DEFINE)
        {
            LOGERROR(stringf("Line %d: Invalid function syntax: function inside function", token->line_number));
            return;
        }

        if (token->special_index >= BARANIUM_KEYWORD_INDEX_DO && token->special_index <= BARANIUM_KEYWORD_INDEX_WHILE)
        {
            baranium_token_parser_parse_loop(&index, token, &function->inner_tokens, &function->tokens, global_tokens);
            continue;
        }

        if (token->special_index == BARANIUM_KEYWORD_INDEX_IF)
        {
            baranium_token_parser_parse_if_statement(&index, token, &function->inner_tokens, &function->tokens, global_tokens);
            continue;
        }

        if (token->special_index == BARANIUM_KEYWORD_INDEX_ELSE)
        {
            LOGERROR(stringf("Line %d: missing `if` for `else` statement", token->line_number));
            return;
        }

        if (token->type == BARANIUM_SOURCE_TOKEN_TYPE_FIELD)
        {
            LOGERROR(stringf("Line %d: Invalid function syntax: fields inside function not allowed", token->line_number));
            return;
        }

        if (baranium_is_internal_type(*token))
        {
            baranium_token_parser_parse_variable(&index, token, &function->inner_tokens, &function->tokens, global_tokens);
            continue;
        }

        baranium_token_parser_parse_expression(&index, token, &function->inner_tokens, &function->tokens, global_tokens);
    }
    // only needed here because the return statement can also be the last token in the function
    baranium_function_token_return_requested(function);

    baranium_token_list_remove_n_at(&function->tokens, parameters_index, function->parameters.count);
}
