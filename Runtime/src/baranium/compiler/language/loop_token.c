#include <baranium/compiler/language/loop_token.h>
#include <baranium/compiler/language/language.h>
#include <baranium/compiler/source_token.h>
#include <baranium/compiler/token_parser.h>
#include <baranium/string_util.h>
#include <baranium/logging.h>

void baranium_loop_token_init(baranium_loop_token* loop)
{
    loop->base.id = BARANIUM_INVALID_INDEX;
    loop->base.type = BARANIUM_TOKEN_TYPE_FUNCTION;
    loop->base.name = NULL;
    loop->t_while = 0;
    loop->t_do_while = 0;
    loop->t_reserved = 0;
    baranium_variable_token_init(&loop->start_variable);
    baranium_expression_token_init(&loop->start_expression);
    baranium_expression_token_init(&loop->condition);
    baranium_expression_token_init(&loop->iteration);
    baranium_source_token_list_init(&loop->inner_tokens);
    baranium_token_list_init(&loop->tokens);
}

void baranium_loop_token_dispose(baranium_loop_token* loop)
{
    baranium_variable_token_dispose(&loop->start_variable);
    baranium_expression_token_dispose(&loop->start_expression);
    baranium_expression_token_dispose(&loop->condition);
    baranium_expression_token_dispose(&loop->iteration);
    baranium_source_token_list_dispose(&loop->inner_tokens);
    baranium_token_list_dispose(&loop->tokens);
}

void baranium_loop_token_parse(baranium_loop_token* loop, baranium_token_list* local_tokens, baranium_token_list* global_tokens)
{
    size_t index = 0;

    size_t local_start_index = loop->tokens.count;

    for (size_t i = 0; i < local_tokens->count; i++)
        baranium_token_list_add(&loop->tokens, local_tokens->data[i]);

    if (loop->start_variable.base.id != BARANIUM_INVALID_INDEX)
        baranium_token_list_add(&loop->tokens, (baranium_token*)&loop->start_variable);

    baranium_expression_token_identify(&loop->condition, &loop->tokens, global_tokens);
    baranium_expression_token_identify(&loop->iteration, &loop->tokens, global_tokens);

    for (; index < loop->inner_tokens.count; index++)
    {
        baranium_source_token* token = baranium_source_token_list_get(&loop->inner_tokens, index);

        if (token->special_index == BARANIUM_KEYWORD_INDEX_DEFINE)
        {
            LOGERROR(stringf("Line %d: Invalid syntax: function definition inside a loop", token->line_number));
            return;
        }

        if (token->special_index >= BARANIUM_KEYWORD_INDEX_DO && token->special_index <= BARANIUM_KEYWORD_INDEX_WHILE)
        {
            baranium_token_parser_parse_loop(&index, token, &loop->inner_tokens, &loop->tokens, global_tokens);
            continue;
        }

        if (token->special_index == BARANIUM_KEYWORD_INDEX_IF)
        {
            baranium_token_parser_parse_if_statement(&index, token, &loop->inner_tokens, &loop->tokens, global_tokens);
            continue;
        }

        if (token->special_index == BARANIUM_KEYWORD_INDEX_ELSE)
        {
            LOGERROR(stringf("Line %d: missing `if` for `else` statement", token->line_number));
            return;
        }

        if (token->type == BARANIUM_SOURCE_TOKEN_TYPE_FIELD)
        {
            LOGERROR(stringf("Line %d: Invalid syntax: fields are not allowed outside of the global scope", token->line_number));
            return;
        }

        if (baranium_is_internal_type(*token))
        {
            ///TODO: make variables inside loops possible through a temporary variable table
            LOGERROR(stringf("Line %d: Invalid syntax: variables are not allowed inside of loops", token->line_number));
            return;
        }

        baranium_token_parser_parse_expression(&index, token, &loop->inner_tokens, &loop->tokens, global_tokens);
    }

    for (size_t i = 0; i < local_tokens->count; i++)
        baranium_token_list_remove_at(&loop->tokens, local_start_index + i);

    if (loop->start_variable.base.id != BARANIUM_INVALID_INDEX)
        baranium_token_list_remove(&loop->tokens, loop->start_variable.base.id);
}

void baranium_loop_token_type_from_source_token(baranium_loop_token* loop, baranium_source_token* loop_identifier)
{
    loop->t_do_while = (loop_identifier->special_index == BARANIUM_KEYWORD_INDEX_DO);
    loop->t_while =   (loop_identifier->special_index == BARANIUM_KEYWORD_INDEX_WHILE || loop_identifier->special_index == BARANIUM_KEYWORD_INDEX_DO);
}
