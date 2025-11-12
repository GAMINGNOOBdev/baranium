#include <baranium/compiler/language/abstract_syntax_tree.h>
#include <baranium/compiler/language/expression_token.h>
#include <baranium/compiler/language/variable_token.h>
#include <baranium/compiler/language/field_token.h>
#include <baranium/compiler/language/language.h>
#include <baranium/compiler/compiler_context.h>
#include <baranium/compiler/language/token.h>
#include <baranium/compiler/token_parser.h>
#include <baranium/compiler/source_token.h>
#include <baranium/string_util.h>
#include <baranium/variable.h>
#include <baranium/logging.h>
#include <memory.h>
#include <stdlib.h>
#include <assert.h>

const char* baranium_expression_type_to_string(baranium_expression_type_t type)
{
    if (type == BARANIUM_EXPRESSION_TYPE_NONE)
        return "None";

    if (type == BARANIUM_EXPRESSION_TYPE_CONDITION)
        return "Condition";

    if (type == BARANIUM_EXPRESSION_TYPE_ASSIGNMENT)
        return "Assignment";

    if (type == BARANIUM_EXPRESSION_TYPE_FUNCTION_CALL)
        return "FunctionCall";

    if (type == BARANIUM_EXPRESSION_TYPE_RETURN_STATEMENT)
        return "ReturnStatement";

    if (type == BARANIUM_EXPRESSION_TYPE_KEYWORD_EXPRESSION)
        return "KeywordExpression";

    if (type == BARANIUM_EXPRESSION_TYPE_ARITHMETIC_OPERATION)
        return "ArithmeticOperation";

    return "Invalid";
}

void baranium_expression_token_init(baranium_expression_token* expression)
{
    memset(expression, 0, sizeof(baranium_expression_token));
    expression->base.id = BARANIUM_INVALID_INDEX;
    expression->base.type = BARANIUM_TOKEN_TYPE_EXPRESSION;
    expression->expression_type = BARANIUM_EXPRESSION_TYPE_INVALID;
    expression->return_type = BARANIUM_VARIABLE_TYPE_INVALID;
    baranium_source_token_list_init(&expression->inner_tokens);
}

void baranium_expression_token_dispose(baranium_expression_token* expression)
{
    baranium_abstract_syntax_tree_node_dispose(expression->ast);
    baranium_source_token_list_dispose(&expression->inner_tokens);
}

void baranium_expression_token_parse_return_statement(baranium_expression_token* expression, baranium_token_list* local_tokens, baranium_token_list* global_tokens)
{
    if (expression->inner_tokens.count < 2)
    {
        expression->return_type = BARANIUM_VARIABLE_TYPE_VOID;
        expression->return_value = NULL;
        return;
    }

    baranium_compiler_context* ctx = baranium_get_compiler_context();
    baranium_source_token_list return_value_list;
    baranium_source_token_list_init(&return_value_list);
    for (size_t i = 1; i < expression->inner_tokens.count; i++)
        baranium_source_token_list_add(&return_value_list, &expression->inner_tokens.data[i]);

    baranium_source_token* valueToken = return_value_list.data;
    expression->return_type = baranium_variable_predict_type(&return_value_list);

    if (expression->return_type != BARANIUM_VARIABLE_TYPE_INVALID && expression->return_type != BARANIUM_VARIABLE_TYPE_VOID)
    {
        expression->return_value = baranium_token_parser_parse_variable_value(&return_value_list, expression->return_type);
        baranium_source_token_list_dispose(&return_value_list);
        return;
    }

    if (expression->return_type == BARANIUM_VARIABLE_TYPE_VOID)
    {
        expression->return_value = NULL;
        baranium_source_token_list_dispose(&return_value_list);
        return;
    }

    if (return_value_list.count > 1)
    {
        expression->return_expression = malloc(sizeof(baranium_expression_token));
        assert(expression->return_expression == NULL);
        expression->return_expression->inner_tokens = return_value_list;
        baranium_expression_token_identify(expression->return_expression, local_tokens, global_tokens);

        return;
    }

    baranium_token* token = baranium_token_lists_contain(valueToken->contents, local_tokens, global_tokens);
    if (token == NULL)
    {
        LOGERROR("Line %d: Invalid return value \"%s\"", valueToken->line_number, valueToken->contents);
        if (ctx)
            ctx->error_occurred = 1;
        baranium_source_token_list_dispose(&return_value_list);
        return;
    }

    expression->return_variable = token->name;

    if (token->type == BARANIUM_TOKEN_TYPE_FIELD)
    {
        baranium_field_token* field = (baranium_field_token*)token;
        expression->return_type = field->type;
        baranium_source_token_list_dispose(&return_value_list);
        return;
    }

    if (token->type == BARANIUM_TOKEN_TYPE_VARIABLE)
    {
        baranium_variable_token* variable = (baranium_variable_token*)token;
        expression->return_type = variable->type;
        baranium_source_token_list_dispose(&return_value_list);
        return;
    }

    LOGERROR("Line %d: Invalid return value \"%s\"", valueToken->line_number, valueToken->contents);
    if (ctx)
        ctx->error_occurred = 1;
    baranium_source_token_list_dispose(&return_value_list);
}

void baranium_expression_token_identify(baranium_expression_token* expression, baranium_token_list* local_tokens, baranium_token_list* global_tokens)
{
    if (expression->inner_tokens.count == 0)
        return;
    
    expression->ast = baranium_abstract_syntax_tree_parse(&expression->inner_tokens);
    expression->expression_type = BARANIUM_EXPRESSION_TYPE_INVALID;
    baranium_abstract_syntax_tree_node* rootNode = expression->ast;
    if (rootNode == NULL)
        return;

    baranium_source_token firstToken = rootNode->contents;
    expression->line_number = firstToken.line_number;

    switch (firstToken.type)
    {
        default:
        case BARANIUM_SOURCE_TOKEN_TYPE_NULL:
        case BARANIUM_SOURCE_TOKEN_TYPE_OROR:
        case BARANIUM_SOURCE_TOKEN_TYPE_ANDAND:
        case BARANIUM_SOURCE_TOKEN_TYPE_NUMBER:
        case BARANIUM_SOURCE_TOKEN_TYPE_EQUALTO:
        case BARANIUM_SOURCE_TOKEN_TYPE_NOTEQUAL:
        case BARANIUM_SOURCE_TOKEN_TYPE_LESSTHAN:
        case BARANIUM_SOURCE_TOKEN_TYPE_LESSEQUAL:
        case BARANIUM_SOURCE_TOKEN_TYPE_GREATERTHAN:
        case BARANIUM_SOURCE_TOKEN_TYPE_GREATEREQUAL:
        {
            expression->expression_type = BARANIUM_EXPRESSION_TYPE_CONDITION;
            break;
        }

        case BARANIUM_SOURCE_TOKEN_TYPE_EQUALSIGN:
        case BARANIUM_SOURCE_TOKEN_TYPE_ANDEQUAL:
        case BARANIUM_SOURCE_TOKEN_TYPE_OREQUAL:
        case BARANIUM_SOURCE_TOKEN_TYPE_DIVEQUAL:
        case BARANIUM_SOURCE_TOKEN_TYPE_MODEQUAL:
        case BARANIUM_SOURCE_TOKEN_TYPE_MULEQUAL:
        case BARANIUM_SOURCE_TOKEN_TYPE_XOREQUAL:
        case BARANIUM_SOURCE_TOKEN_TYPE_PLUSEQUAL:
        case BARANIUM_SOURCE_TOKEN_TYPE_MINUSEQUAL:
        {
            expression->expression_type = BARANIUM_EXPRESSION_TYPE_ASSIGNMENT;
            break;
        }

        case BARANIUM_SOURCE_TOKEN_TYPE_PLUS:
        case BARANIUM_SOURCE_TOKEN_TYPE_PLUSPLUS:
        case BARANIUM_SOURCE_TOKEN_TYPE_MINUS:
        case BARANIUM_SOURCE_TOKEN_TYPE_MINUSMINUS:
        case BARANIUM_SOURCE_TOKEN_TYPE_MODULO:
        case BARANIUM_SOURCE_TOKEN_TYPE_ASTERISK:
        case BARANIUM_SOURCE_TOKEN_TYPE_SLASH:
        case BARANIUM_SOURCE_TOKEN_TYPE_AND:
        case BARANIUM_SOURCE_TOKEN_TYPE_OR:
        case BARANIUM_SOURCE_TOKEN_TYPE_CARET:
        {
            expression->expression_type = BARANIUM_EXPRESSION_TYPE_ARITHMETIC_OPERATION;
            break;
        }

        case BARANIUM_SOURCE_TOKEN_TYPE_TEXT:
        {
            expression->expression_type = BARANIUM_EXPRESSION_TYPE_CONDITION;

            if (rootNode->sub_nodes.count != 0)
                expression->expression_type = BARANIUM_EXPRESSION_TYPE_FUNCTION_CALL;

            break;
        }

        case BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD:
        {
            if (firstToken.special_index == BARANIUM_KEYWORD_INDEX_RETURN)
            {
                expression->expression_type = BARANIUM_EXPRESSION_TYPE_RETURN_STATEMENT;
                baranium_expression_token_parse_return_statement(expression, local_tokens, global_tokens);
                break;
            }

            if (firstToken.special_index == BARANIUM_KEYWORD_INDEX_BREAK || firstToken.special_index == BARANIUM_KEYWORD_INDEX_CONTINUE)
            {
                expression->expression_type = BARANIUM_EXPRESSION_TYPE_KEYWORD_EXPRESSION;
                expression->line_number = firstToken.line_number;
                break;
            }

            if (expression->inner_tokens.count < 3 && firstToken.special_index >= BARANIUM_KEYWORD_INDEX_INSTANTIATE && firstToken.special_index <= BARANIUM_KEYWORD_INDEX_DETACH)
            {
                baranium_source_token* object_source_token = baranium_source_token_list_get(&expression->inner_tokens, 1);

                baranium_token* parsed_object_token = baranium_token_lists_contain(object_source_token->contents, local_tokens, global_tokens);
                if (parsed_object_token == NULL && !(strcmp(object_source_token->contents, "null") == 0 || strcmp(object_source_token->contents, baranium_keywords[BARANIUM_KEYWORD_INDEX_ATTACHED].name) == 0))
                {
                    LOGERROR("Line %d: Cannot parse keyword expression: Cannot find variable named '%s'", firstToken.line_number, object_source_token->contents);
                    baranium_compiler_context* ctx = baranium_get_compiler_context();
                    if (ctx)
                        ctx->error_occurred = 1;
                    return;
                }

                expression->expression_type = BARANIUM_EXPRESSION_TYPE_KEYWORD_EXPRESSION;
                expression->return_type = BARANIUM_VARIABLE_TYPE_OBJECT;
                expression->return_value = object_source_token->contents;
                expression->line_number = object_source_token->line_number;
                break;
            }

            break;
        }
    }
}
