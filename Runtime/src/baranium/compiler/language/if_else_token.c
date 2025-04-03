#include "baranium/compiler/language/language.h"
#include <baranium/compiler/language/expression_token.h>
#include <baranium/compiler/language/if_else_token.h>
#include <baranium/compiler/language/token.h>
#include <baranium/compiler/source_token.h>
#include <baranium/compiler/token_parser.h>
#include <baranium/string_util.h>
#include <baranium/logging.h>

void baranium_if_else_token_init(baranium_if_else_token* ifelsetoken)
{
    ifelsetoken->base.type = BARANIUM_TOKEN_TYPE_IFELSESTATEMENT;
    ifelsetoken->base.id = BARANIUM_INVALID_INDEX;
    ifelsetoken->has_else_statement = 0;
    baranium_expression_token_init(&ifelsetoken->condition);
    baranium_source_token_list_init(&ifelsetoken->inner_tokens);
    baranium_token_list_init(&ifelsetoken->tokens);
    baranium_token_list_init(&ifelsetoken->chained_statements);
}

void baranium_if_else_token_dispose(baranium_if_else_token* ifelsetoken)
{
    baranium_expression_token_dispose(&ifelsetoken->condition);
    baranium_source_token_list_dispose(&ifelsetoken->inner_tokens, 0);
    baranium_token_list_dispose(&ifelsetoken->tokens);
    baranium_token_list_dispose(&ifelsetoken->chained_statements);
}

void baranium_if_else_token_parse(baranium_if_else_token* ifelsetoken, baranium_token_list* local_tokens, baranium_token_list* global_tokens)
{
    size_t index = 0;

    size_t local_start_index = ifelsetoken->tokens.count;

    for (size_t i = 0; i < local_tokens->count; i++)
        baranium_token_list_add(&ifelsetoken->tokens, local_tokens->data[i]);

    for (; index+1 < ifelsetoken->inner_tokens.count; index++)
    {
        baranium_source_token* token = baranium_source_token_list_get(&ifelsetoken->inner_tokens, index);

        if (token->special_index == BARANIUM_KEYWORD_INDEX_DEFINE)
        {
            baranium_token_parser_parse_function(&index, token, &ifelsetoken->inner_tokens, &ifelsetoken->tokens, global_tokens);
            continue;
        }

        if (token->special_index == BARANIUM_KEYWORD_INDEX_IF)
        {
            baranium_token_parser_parse_if_statement(&index, token, &ifelsetoken->inner_tokens, &ifelsetoken->tokens, global_tokens);
            continue;
        }

        if (token->special_index == BARANIUM_KEYWORD_INDEX_ELSE)
        {
            LOGERROR(stringf("Line %d: missing `if` for `else` statement", token->line_number));
            return;
        }

        if (token->type == BARANIUM_SOURCE_TOKEN_TYPE_FIELD)
        {
            baranium_token_parser_parse_field(&index, token, &ifelsetoken->inner_tokens, &ifelsetoken->tokens, global_tokens);
            continue;
        }
 
        if (baranium_is_internal_type(*token))
        {
            baranium_token_parser_parse_variable(&index, token, &ifelsetoken->inner_tokens, &ifelsetoken->tokens, global_tokens);
            continue;
        }

        baranium_token_parser_parse_expression(&index, token, &ifelsetoken->inner_tokens, &ifelsetoken->tokens, global_tokens);
    }

    baranium_token_list_remove_n_at(&ifelsetoken->tokens, local_start_index, local_tokens->count);
}
