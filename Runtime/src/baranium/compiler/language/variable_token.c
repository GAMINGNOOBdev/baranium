#include <baranium/compiler/language/expression_token.h>
#include <baranium/compiler/language/variable_token.h>
#include <baranium/compiler/language/language.h>
#include <baranium/compiler/language/token.h>
#include <baranium/compiler/source_token.h>
#include <baranium/variable.h>
#include <baranium/defines.h>
#include <string.h>

baranium_variable_type_t baranium_variable_type_from_token(baranium_source_token* token)
{
    if (!baranium_is_internal_type(*token))
        return BARANIUM_VARIABLE_TYPE_INVALID;

    if (token->special_index == BARANIUM_KEYWORD_INDEX_OBJECT)
        return BARANIUM_VARIABLE_TYPE_OBJECT;
    if (token->special_index == BARANIUM_KEYWORD_INDEX_STRING)
        return BARANIUM_VARIABLE_TYPE_STRING;
    if (token->special_index == BARANIUM_KEYWORD_INDEX_FLOAT)
        return BARANIUM_VARIABLE_TYPE_FLOAT;
    if (token->special_index == BARANIUM_KEYWORD_INDEX_BOOL)
        return BARANIUM_VARIABLE_TYPE_BOOL;
    if (token->special_index == BARANIUM_KEYWORD_INDEX_INT32)
        return BARANIUM_VARIABLE_TYPE_INT32;
    if (token->special_index == BARANIUM_KEYWORD_INDEX_UINT32)
        return BARANIUM_VARIABLE_TYPE_UINT32;
    if (token->special_index == BARANIUM_KEYWORD_INDEX_DOUBLE)
        return BARANIUM_VARIABLE_TYPE_DOUBLE;
    if (token->special_index == BARANIUM_KEYWORD_INDEX_INT8)
        return BARANIUM_VARIABLE_TYPE_INT8;
    if (token->special_index == BARANIUM_KEYWORD_INDEX_UINT8)
        return BARANIUM_VARIABLE_TYPE_UINT8;
    if (token->special_index == BARANIUM_KEYWORD_INDEX_INT16)
        return BARANIUM_VARIABLE_TYPE_INT16;
    if (token->special_index == BARANIUM_KEYWORD_INDEX_UINT16)
        return BARANIUM_VARIABLE_TYPE_UINT16;
    if (token->special_index == BARANIUM_KEYWORD_INDEX_INT64)
        return BARANIUM_VARIABLE_TYPE_INT64;
    if (token->special_index == BARANIUM_KEYWORD_INDEX_UINT64)
        return BARANIUM_VARIABLE_TYPE_UINT64;
    if (token->special_index == BARANIUM_KEYWORD_INDEX_VOID)
        return BARANIUM_VARIABLE_TYPE_VOID;

    return BARANIUM_VARIABLE_TYPE_INVALID;
}

baranium_variable_type_t baranium_variable_predict_type(baranium_source_token_list* tokens)
{
    size_t tokenIndex = 0;

    baranium_source_token* first_token = baranium_source_token_list_get(tokens, tokenIndex++);

    if (first_token->special_index == BARANIUM_KEYWORD_INDEX_TRUE || first_token->special_index == BARANIUM_KEYWORD_INDEX_FALSE)
        return BARANIUM_VARIABLE_TYPE_BOOL;

    if (first_token->type == BARANIUM_SOURCE_TOKEN_TYPE_DOUBLEQUOTE)
        return BARANIUM_VARIABLE_TYPE_STRING;

    if (first_token->type == BARANIUM_SOURCE_TOKEN_TYPE_MINUS || first_token->type == BARANIUM_SOURCE_TOKEN_TYPE_PLUS)
    {
        if (tokenIndex >= tokens->count)
            return BARANIUM_VARIABLE_TYPE_INVALID;

        baranium_source_token* secondToken = baranium_source_token_list_get(tokens, tokenIndex++);
        if (secondToken->type == BARANIUM_SOURCE_TOKEN_TYPE_NUMBER)
        {
            if (tokenIndex+1 < tokens->count)
            {
                baranium_source_token* thirdToken = baranium_source_token_list_get(tokens, tokenIndex++);
                if (thirdToken->type == BARANIUM_SOURCE_TOKEN_TYPE_DOT)
                    return BARANIUM_VARIABLE_TYPE_FLOAT;
            }
            return first_token->type == BARANIUM_SOURCE_TOKEN_TYPE_MINUS ? BARANIUM_VARIABLE_TYPE_INT32 : BARANIUM_VARIABLE_TYPE_UINT32;
        }
    }

    if (first_token->type == BARANIUM_SOURCE_TOKEN_TYPE_NUMBER)
    {
        if (tokenIndex >= tokens->count)
            return BARANIUM_VARIABLE_TYPE_UINT32;
        
        baranium_source_token* secondToken = baranium_source_token_list_get(tokens, tokenIndex++);
        if (secondToken->type == BARANIUM_SOURCE_TOKEN_TYPE_DOT)
            return BARANIUM_VARIABLE_TYPE_FLOAT;
    }

    if (first_token->type == BARANIUM_SOURCE_TOKEN_TYPE_NULL)
        return BARANIUM_VARIABLE_TYPE_VOID;

    return BARANIUM_VARIABLE_TYPE_INVALID;
}

void baranium_variable_token_init(baranium_variable_token* variable)
{
    memset(variable, 0, sizeof(baranium_variable_token));
    variable->base.id = BARANIUM_INVALID_INDEX;
    variable->base.type = BARANIUM_TOKEN_TYPE_VARIABLE;
    variable->type = BARANIUM_VARIABLE_TYPE_INVALID;
    variable->array_size = -1;
    baranium_expression_token_init(&variable->init_expression);
}

void baranium_variable_token_dispose(baranium_variable_token* variable)
{
    if (!variable)
        return;

    baranium_expression_token_dispose(&variable->init_expression);
}
