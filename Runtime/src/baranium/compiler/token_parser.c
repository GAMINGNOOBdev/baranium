#include <baranium/compiler/language/abstract_syntax_tree.h>
#include <baranium/compiler/language/expression_token.h>
#include <baranium/compiler/language/function_token.h>
#include <baranium/compiler/language/variable_token.h>
#include <baranium/compiler/language/if_else_token.h>
#include <baranium/compiler/language/field_token.h>
#include <baranium/compiler/language/loop_token.h>
#include <baranium/compiler/language/language.h>
#include <baranium/compiler/language/token.h>
#include <baranium/compiler/source_token.h>
#include <baranium/compiler/token_parser.h>
#include <baranium/compiler/source.h>
#include <baranium/string_util.h>
#include <baranium/variable.h>
#include <baranium/runtime.h>
#include <baranium/logging.h>
#include <stdlib.h>
#include <memory.h>

void baranium_token_parser_init(baranium_token_parser* parser)
{
    if (parser == NULL)
        return;

    baranium_token_list_init(&parser->tokens);
}

void baranium_token_parser_parse(baranium_token_parser* parser, baranium_source_token_list* tokens)
{
    if (parser == NULL)
        return;

    size_t index = 0;

    baranium_abstract_syntax_tree_init();

    for (; index+1 < tokens->count; index++)
    {
        baranium_source_token* token = baranium_source_token_list_get(tokens, index);

        if (token->special_index == BARANIUM_KEYWORD_INDEX_DEFINE)
        {
            baranium_token_parser_parse_function(&index, token, tokens, &parser->tokens, NULL);
            continue;
        }

        if (token->special_index >= BARANIUM_KEYWORD_INDEX_DO && token->special_index <= BARANIUM_KEYWORD_INDEX_WHILE)
        {
            LOGERROR("Line %d: cannot have loops in the global scope", token->line_number);
            return;
        }

        if (token->special_index == BARANIUM_KEYWORD_INDEX_IF)
        {
            LOGERROR("Line %d: cannot have `if` in the global scope", token->line_number);
            return;
        }

        if (token->special_index == BARANIUM_KEYWORD_INDEX_ELSE)
        {
            LOGERROR("Line %d: cannot have `else` in the global scope", token->line_number);
            return;
        }

        if (token->type == BARANIUM_SOURCE_TOKEN_TYPE_FIELD)
        {
            baranium_token_parser_parse_field(&index, token, tokens, &parser->tokens, NULL);
            continue;
        }

        if (baranium_is_internal_type(*token))
        {
            baranium_token_parser_parse_variable(&index, token, tokens, &parser->tokens, NULL);
            continue;
        }

        LOGERROR("Line %d: cannot have expressions in the global scope (%s)", token->line_number, token->contents);
        return;
    }

    baranium_abstract_syntax_tree_dispose();
}

void baranium_token_parser_dispose(baranium_token_parser* parser)
{
    if (parser == NULL)
        return;

    baranium_token_list_dispose(&parser->tokens);
}

void baranium_token_parser_parse_variable(size_t* index, baranium_source_token* current, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens)
{
    baranium_variable_token* variable = (baranium_variable_token*)malloc(sizeof(baranium_variable_token));
    baranium_variable_token_init(variable);
    variable->type = baranium_variable_type_from_token(current);

    if (variable->type == BARANIUM_VARIABLE_TYPE_INVALID)
    {
        LOGERROR("Line %d: Inalid variable type '%s'", current->line_number, current->contents);
        baranium_variable_token_dispose(variable);
        free(variable);
        return;
    }

    (*index)++;
    baranium_source_token* nameToken = baranium_source_token_list_get(tokens, *index);
    if (nameToken->type == BARANIUM_SOURCE_TOKEN_TYPE_BRACKETOPEN && baranium_source_token_list_get(tokens, (*index)+1)->type == BARANIUM_SOURCE_TOKEN_TYPE_BRACKETCLOSE)
    {
        (*index)+=2;
        nameToken = baranium_source_token_list_get(tokens, *index);
        variable->array_size = 0;
    }

    if (nameToken->type == BARANIUM_SOURCE_TOKEN_TYPE_TEXT)
        variable->base.name = nameToken->contents;
    else
    {
        LOGERROR("Line %d: No valid name has been prodived for variable", nameToken->line_number);
        baranium_variable_token_dispose(variable);
        free(variable);
        return;
    }

    if (baranium_token_lists_contain(nameToken->contents, output, global_tokens) != NULL)
    {
        LOGERROR("Line %d: Name \"%s\" is already occupied", nameToken->line_number, nameToken->contents);
        baranium_variable_token_dispose(variable);
        free(variable);
        return;
    }

    variable->base.id = baranium_get_id_of_name(variable->base.name);

    (*index)++;
    baranium_source_token* nextToken = baranium_source_token_list_get(tokens, *index);
    if (nextToken->type == BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON)
        goto end;
    else if (nextToken->type != BARANIUM_SOURCE_TOKEN_TYPE_EQUALSIGN)
    {
        LOGERROR("Line %d: Invalid syntax for variable definition/assignment", nextToken->line_number);
        baranium_variable_token_dispose(variable);
        free(variable);
        return;
    }
    baranium_source_token_list_add(&variable->init_expression.inner_tokens, nameToken);
    baranium_source_token_list_add(&variable->init_expression.inner_tokens, nextToken);

    while (nextToken->type != BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON && (*index)+1 < tokens->count)
    {
        (*index)++;
        nextToken = baranium_source_token_list_get(tokens, *index);
        baranium_source_token_list_add(&variable->init_expression.inner_tokens, nextToken);
    }

    // to remove the last pushed semicolon
    baranium_source_token_list_pop_token(&variable->init_expression.inner_tokens);

end:
    baranium_expression_token_identify(&variable->init_expression, output, global_tokens);
    baranium_token_list_add(output, (baranium_token*)variable);
}

void baranium_token_parser_parse_field(size_t* index, baranium_source_token* _, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens)
{
    (*index)++;
    baranium_source_token* typeToken = baranium_source_token_list_get(tokens, *index);
    baranium_field_token* field = (baranium_field_token*)malloc(sizeof(baranium_field_token));
    baranium_field_token_init(field);
    field->type = baranium_variable_type_from_token(typeToken);
    (*index)++;
    baranium_source_token* nameToken = baranium_source_token_list_get(tokens, *index);
    if (nameToken->type == BARANIUM_SOURCE_TOKEN_TYPE_BRACKETOPEN && baranium_source_token_list_get(tokens, (*index)+1) == nameToken)
    {
        index+=2;
        nameToken = baranium_source_token_list_get(tokens, *index);
        field->array_size = 0;
    }

    if (nameToken->type == BARANIUM_SOURCE_TOKEN_TYPE_TEXT)
        field->base.name = nameToken->contents;
    else
    {
        LOGERROR("Line %d: No valid name has been prodived for field", nameToken->line_number);
        baranium_field_token_dispose(field);
        free(field);
        return;
    }

    field->base.id = baranium_get_id_of_name(field->base.name);

    (*index)++;
    baranium_source_token* nextToken = baranium_source_token_list_get(tokens, *index);
    if (nextToken->type == BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON)
    {
        field->value = NULL;
        goto end;
    }
    else if (nextToken->type != BARANIUM_SOURCE_TOKEN_TYPE_EQUALSIGN)
    {
        LOGERROR("Line %d: Invalid syntax for field definition/assignment", nextToken->line_number);
        baranium_field_token_dispose(field);
        free(field);
        return;
    }
    baranium_source_token_list_add(&field->init_expression.inner_tokens, nameToken);
    baranium_source_token_list_add(&field->init_expression.inner_tokens, nextToken);

    while (nextToken->type != BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON && (*index)+1 < tokens->count)
    {
        (*index)++;
        nextToken = baranium_source_token_list_get(tokens, *index);
        baranium_source_token_list_add(&field->init_expression.inner_tokens, nextToken);
    }

    // to remove the last pushed semicolon
    baranium_source_token_list_pop_token(&field->init_expression.inner_tokens);

end:
baranium_expression_token_identify(&field->init_expression, output, global_tokens);
    baranium_token_list_add(output, (baranium_token*)field);
}

uint8_t baranium_token_parser_parse_expression(size_t* index, baranium_source_token* _, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens)
{
    baranium_expression_token* expression = (baranium_expression_token*)malloc(sizeof(baranium_expression_token));
    baranium_expression_token_init(expression);

    baranium_source_token* nextToken = baranium_source_token_list_get(tokens, *index);
    baranium_source_token_list_add(&expression->inner_tokens, nextToken);
    if (!baranium_token_parser_parse_content_using_depth(index, BARANIUM_SOURCE_TOKEN_TYPE_INVALID, BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON, tokens, &expression->inner_tokens))
    {
        LOGERROR("Line %d: Expected a ';' at the end", nextToken->line_number);
        return 0;
    }

    baranium_expression_token_identify(expression, output, global_tokens);

    baranium_token_list_add(output, (baranium_token*)expression);

    return expression->expression_type == BARANIUM_EXPRESSION_TYPE_RETURN_STATEMENT;
}

void baranium_token_parser_parse_function(size_t* index, baranium_source_token* _, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens)
{
    baranium_function_token* function = (baranium_function_token*)malloc(sizeof(baranium_function_token));
    baranium_function_token_init(function);

    (*index)++;
    baranium_source_token* nameToken = baranium_source_token_list_get(tokens, *index);
    if (nameToken->type != BARANIUM_SOURCE_TOKEN_TYPE_TEXT)
    {
        LOGERROR("Line %d: No valid name has been prodived for function", nameToken->line_number);
        return;
    }

    if (baranium_token_lists_contain(nameToken->contents, output, global_tokens) != NULL)
    {
        LOGERROR("Line %d: Name \"%s\" is already occupied", nameToken->line_number, nameToken->contents);
        return;
    }

    function->base.name = nameToken->contents;
    function->base.id = baranium_get_id_of_name(function->base.name);

    (*index)++;
    baranium_source_token* parametersStart = baranium_source_token_list_get(tokens, *index);

    if (parametersStart->type != BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN)
    {
        LOGERROR("Line %d: Invalid function syntax", parametersStart->line_number);
        return;
    }

    (*index)++;
    baranium_source_token* parameter = baranium_source_token_list_get(tokens, *index);
    if (parameter->type == BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISCLOSE)
    {
        goto functionReadContents;
    }
    if (!baranium_is_internal_type(*parameter))
    {
        LOGERROR("Line %d: Invalid function parameter type '%s'", parameter->line_number, parameter->contents);
        return;
    }

    while (parameter->type != BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISCLOSE && (*index)+1 < tokens->count)
        baranium_token_parser_parse_function_parameter(index, function, parameter, tokens);

functionReadContents:

    (*index)++;
    baranium_source_token* functionContents = baranium_source_token_list_get(tokens, *index);
    if (functionContents->type == BARANIUM_SOURCE_TOKEN_TYPE_EQUALSIGN)
    {
        (*index)++;
        functionContents = baranium_source_token_list_get(tokens, *index);
        function->return_type = baranium_variable_type_from_token(functionContents);
        (*index)++;
        functionContents = baranium_source_token_list_get(tokens, *index);
    }
    if (functionContents->type == BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON)
    {
        function->only_declaration = 1;
        baranium_token_list_add(output, (baranium_token*)function);
        return;
    }

    if (functionContents->type != BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETOPEN)
    {
        LOGERROR("Line %d: Invalid function syntax", functionContents->line_number);
        return;
    }

    if (!baranium_token_parser_parse_content_using_depth(index, BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETOPEN, BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETCLOSE, tokens, &function->inner_tokens))
    {
        LOGERROR("Line %d: Invalid contents for function contents, invalid content depth", functionContents->line_number);
        return;
    }

    baranium_function_token_parse(function, output); // since functions can only exist in the global space, passing output as global tokens is reasonable

    baranium_token_list_add(output, (baranium_token*)function);
}

void baranium_token_parser_parse_if_statement(size_t* index, baranium_source_token* _, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens)
{
    (*index)++;
    baranium_source_token* conditionStart = baranium_source_token_list_get(tokens, *index);
    if (conditionStart->type != BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN)
    {
        LOGERROR("Line %d: Invalid start of if-statement, expected '(', got '%s'", conditionStart->line_number, conditionStart->contents);
        return;
    }

    baranium_if_else_token* ifElseStatement = (baranium_if_else_token*)malloc(sizeof(baranium_if_else_token));
    baranium_if_else_token* alternativeCondition;
    baranium_if_else_token* elseStatement;

    baranium_if_else_token_init(ifElseStatement);

    (*index)++;
    baranium_source_token* nextToken = baranium_source_token_list_get(tokens, *index);
    if (nextToken->type != BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN)
        baranium_source_token_list_add(&ifElseStatement->condition.inner_tokens, nextToken);
    else (*index)--;

    if (!baranium_token_parser_parse_content_using_depth(index, BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN, BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISCLOSE,
                               tokens, &ifElseStatement->condition.inner_tokens))
    {
        LOGERROR("Line %d: Invalid condition contents for if-statement", nextToken->line_number);
        return;
    }

    baranium_expression_token_identify(&ifElseStatement->condition, output, global_tokens);

    (*index)++;
    nextToken = baranium_source_token_list_get(tokens, *index);
    if (nextToken->type == BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETOPEN)
        goto readStatementContents;

    baranium_source_token_list_add(&ifElseStatement->condition.inner_tokens, nextToken);
    while (nextToken->type != BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON && (*index)+1 < tokens->count)
    {
        (*index)++;
        nextToken = baranium_source_token_list_get(tokens, *index);
        baranium_source_token_list_add(&ifElseStatement->condition.inner_tokens, nextToken);
    }

    goto readNextPart;

readStatementContents:
    if (!baranium_token_parser_parse_content_using_depth(index, BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETOPEN, BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETCLOSE, tokens, &ifElseStatement->inner_tokens))
    {
        nextToken = baranium_source_token_list_get(tokens, *index);
        LOGERROR("Line %d: Invalid contents for if-statement, invalid content depth", nextToken->line_number);
        return;
    }

    goto readNextPart;

readNextPart:
    if ((*index)+1 < tokens->count)
    {
        if (baranium_source_token_list_get(tokens, (*index)+1)->special_index == BARANIUM_KEYWORD_INDEX_ELSE)
        {
            (*index)++;
            goto readElse;
        }
    }

    goto end;

readAlternativeConditions:
    (*index)++;
    nextToken = baranium_source_token_list_get(tokens, *index);
    if (nextToken->type != BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN)
    {
        LOGERROR("Line %d: Invalid start of if-statement, expected '(', got '%s'", conditionStart->line_number, conditionStart->contents);
        return;
    }

    alternativeCondition = (baranium_if_else_token*)malloc(sizeof(baranium_if_else_token));
    baranium_if_else_token_init(alternativeCondition);
    if (!baranium_token_parser_parse_content_using_depth(index, BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN, BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISCLOSE, tokens, &alternativeCondition->condition.inner_tokens))
    {
        LOGERROR("Line %d: Invalid condition contents for if-statement", nextToken->line_number);
        return;
    }

    baranium_expression_token_identify(&alternativeCondition->condition, output, global_tokens);

    (*index)++;
    nextToken = baranium_source_token_list_get(tokens, *index);
    if (nextToken->type == BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETOPEN)
        goto readAlternaticeConditionContents;

    baranium_source_token_list_add(&alternativeCondition->inner_tokens, nextToken);
    while (nextToken->type != BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON && (*index)+1 < tokens->count)
    {
        (*index)++;
        nextToken = baranium_source_token_list_get(tokens, *index);
        baranium_source_token_list_add(&alternativeCondition->inner_tokens, nextToken);
    }

    goto endAlternateCondition;

readAlternaticeConditionContents:
    if (!baranium_token_parser_parse_content_using_depth(index, BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETOPEN, BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETCLOSE, tokens, &alternativeCondition->inner_tokens))
    {
        nextToken = baranium_source_token_list_get(tokens, *index);
        LOGERROR("Line %d: Invalid contents for else-if-statement, invalid content depth", nextToken->line_number);
        return;
    }
    baranium_if_else_token_parse(alternativeCondition, output, global_tokens);

    goto endAlternateCondition;

endAlternateCondition:
    baranium_token_list_add(&ifElseStatement->chained_statements, (baranium_token*)alternativeCondition);

    goto readNextPart;

readElse:
    (*index)++;
    nextToken = baranium_source_token_list_get(tokens, *index);
    if (nextToken->special_index == BARANIUM_KEYWORD_INDEX_IF)
        goto readAlternativeConditions;

    ifElseStatement->has_else_statement = 1;
    elseStatement = (baranium_if_else_token*)malloc(sizeof(baranium_if_else_token));
    baranium_if_else_token_init(elseStatement);

    if (nextToken->type == BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETOPEN)
        goto readElseContents;

    baranium_source_token_list_add(&elseStatement->inner_tokens, nextToken);
    while (nextToken->type != BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON && (*index)+1 < tokens->count)
    {
        (*index)++;
        nextToken = baranium_source_token_list_get(tokens, *index);
        baranium_source_token_list_add(&elseStatement->inner_tokens, nextToken);
    }
    goto readElseEnd;

readElseContents:
    if (!baranium_token_parser_parse_content_using_depth(index, BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETOPEN, BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETCLOSE, tokens, &elseStatement->inner_tokens))
    {
        nextToken = baranium_source_token_list_get(tokens, *index);
        LOGERROR("Line %d: Invalid contents for else-statement, invalid content depth", nextToken->line_number);
        return;
    }

    goto readElseEnd;

readElseEnd:
    elseStatement->condition.base.type = BARANIUM_TOKEN_TYPE_INVALID;
    baranium_if_else_token_parse(elseStatement, output, global_tokens);
    baranium_token_list_add(&ifElseStatement->chained_statements, (baranium_token*)elseStatement);

end:
    baranium_if_else_token_parse(ifElseStatement, output, global_tokens);

    baranium_token_list_add(output, (baranium_token*)ifElseStatement);
}

void baranium_token_parser_parse_do_while_loop(baranium_loop_token* loop, size_t* index, baranium_source_token* current, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens);
void baranium_token_parser_parse_while_loop(baranium_loop_token* loop, size_t* index, baranium_source_token* current, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens);
void baranium_token_parser_parse_for_loop(baranium_loop_token* loop, size_t* index, baranium_source_token* current, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens);

void baranium_token_parser_parse_loop(size_t* index, baranium_source_token* current, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens)
{
    baranium_loop_token* loop = (baranium_loop_token*)malloc(sizeof(baranium_loop_token));
    baranium_loop_token_init(loop);
    baranium_loop_token_type_from_source_token(loop, current);
    (*index)++;

    if (current->special_index == BARANIUM_KEYWORD_INDEX_DO)
    {
        baranium_token_parser_parse_do_while_loop(loop, index, current, tokens, output, global_tokens);
        loop->base.type = BARANIUM_TOKEN_TYPE_DOWHILELOOP;
    }

    if (current->special_index == BARANIUM_KEYWORD_INDEX_FOR)
    {
        baranium_token_parser_parse_for_loop(loop, index, current, tokens, output, global_tokens);
        loop->base.type = BARANIUM_TOKEN_TYPE_FORLOOP;
    }

    if (current->special_index == BARANIUM_KEYWORD_INDEX_WHILE)
    {
        baranium_token_parser_parse_while_loop(loop, index, current, tokens, output, global_tokens);
        loop->base.type = BARANIUM_TOKEN_TYPE_WHILELOOP;
    }

    baranium_token_list_add(output, (baranium_token*)loop);
}

void baranium_token_parser_parse_function_parameter(size_t* index, baranium_function_token* function, baranium_source_token* current, baranium_source_token_list* tokens)
{
    baranium_variable_token* parameter = (baranium_variable_token*)malloc(sizeof(baranium_variable_token));
    baranium_variable_token_init(parameter);
    parameter->type = baranium_variable_type_from_token(current);
    (*index)++;
    baranium_source_token* nameToken = baranium_source_token_list_get(tokens, *index);
    if (nameToken->type != BARANIUM_SOURCE_TOKEN_TYPE_TEXT)
    {
        LOGERROR("Line %d: No valid name ('%s') has been prodived for parameter in function definition", nameToken->line_number, nameToken->contents);
        return;
    }

    parameter->base.name = nameToken->contents;
    parameter->base.id = baranium_get_id_of_name(parameter->base.name);

    (*index)++;
    baranium_source_token* nextToken = baranium_source_token_list_get(tokens, *index);
    if (nextToken->type == BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISCLOSE || nextToken->type == BARANIUM_SOURCE_TOKEN_TYPE_COMMA)
    {
        parameter->value = "";
        baranium_token_list_add(&function->parameters, (baranium_token*)parameter);

        *current = *nextToken;
        if (nextToken->type == BARANIUM_SOURCE_TOKEN_TYPE_COMMA)
        {
            (*index)++;
            baranium_source_token* nextTypeToken = baranium_source_token_list_get(tokens, *index);
            *current = *nextTypeToken;
        }
        return;
    }

    LOGERROR("Line %d: Invalid syntax for parameter definition in function definition", nextToken->line_number);
}

const char* baranium_token_parser_parse_variable_value(baranium_source_token_list* tokens, baranium_variable_type_t varType)
{
    size_t tokenIndex = 0;

    if (varType == BARANIUM_VARIABLE_TYPE_INVALID || tokens->count == 0)
        return "null";

    if (varType == BARANIUM_VARIABLE_TYPE_OBJECT)
    {
        baranium_source_token* dataToken = baranium_source_token_list_get(tokens, tokenIndex);
        tokenIndex++;

        if (dataToken->special_index == BARANIUM_KEYWORD_INDEX_ATTACHED)
            return "attached";

        if (dataToken->type == BARANIUM_SOURCE_TOKEN_TYPE_TEXT)
            return dataToken->contents;

        if (dataToken->type == BARANIUM_SOURCE_TOKEN_TYPE_NULL)
            return "null";

        LOGERROR("Line %d: Invalid object assignment", dataToken->line_number);
    }

    if (varType == BARANIUM_VARIABLE_TYPE_STRING)
    {
        baranium_source_token* stringStart = baranium_source_token_list_get(tokens, tokenIndex);

        if (stringStart->type == BARANIUM_SOURCE_TOKEN_TYPE_NULL)
            return "";

        if (stringStart->type != BARANIUM_SOURCE_TOKEN_TYPE_DOUBLEQUOTE)
        {
            LOGERROR("Line %d: Invalid string assignment", stringStart->line_number);
            return NULL;
        }

        tokenIndex++;
        baranium_source_token* contentsToken = baranium_source_token_list_get(tokens, tokenIndex);
        if (contentsToken->type == BARANIUM_SOURCE_TOKEN_TYPE_DOUBLEQUOTE && tokens->count == 2)
            return "";

        tokenIndex++;
        baranium_source_token* stringEnd = baranium_source_token_list_get(tokens, tokenIndex);
        tokenIndex++;

        if (stringEnd->type != BARANIUM_SOURCE_TOKEN_TYPE_DOUBLEQUOTE)
        {
            LOGERROR("Line %d: Invalid string assignment", contentsToken->line_number);
            return NULL;
        }

        return contentsToken->contents;
    }

    if (varType == BARANIUM_VARIABLE_TYPE_FLOAT)
    {
        baranium_source_token* valueStart = baranium_source_token_list_get(tokens, tokenIndex);
        tokenIndex++;

        if (valueStart->type == BARANIUM_SOURCE_TOKEN_TYPE_NULL)
            return "0";

        if (tokenIndex >= tokens->count)
        {
            if (valueStart->type == BARANIUM_SOURCE_TOKEN_TYPE_NUMBER)
                return valueStart->contents;
            
            LOGERROR("Line %d: Invalid float assignment: Not a number", valueStart->line_number);
            return NULL;
        }

        baranium_source_token* dotPart = baranium_source_token_list_get(tokens, tokenIndex);
        tokenIndex++;

        if (valueStart->type == BARANIUM_SOURCE_TOKEN_TYPE_NUMBER && dotPart->type == BARANIUM_SOURCE_TOKEN_TYPE_DOT)
        {
            const char* decimal = "0";
            if (tokenIndex+1 < tokens->count)
            {
                baranium_source_token* decimalPart = baranium_source_token_list_get(tokens, tokenIndex);
                if (decimalPart->type != BARANIUM_SOURCE_TOKEN_TYPE_NUMBER)
                {
                    LOGERROR("Line %d: Invalid float assignment: Invalid literal after dot", decimalPart->line_number);
                    return NULL;
                }
                decimal = decimalPart->contents;
            }

            size_t oldlen = strlen(valueStart->contents);
            size_t newlen = strlen(valueStart->contents) + strlen(decimal) + 1;
            valueStart->contents = realloc(valueStart->contents, newlen+1);
            valueStart->contents[newlen] = 0;
            valueStart->contents[oldlen++] = '.';
            for (size_t i = 0; i < strlen(decimal); i++)
                valueStart->contents[oldlen + i] = decimal[i];
            return valueStart->contents;
        }
        if (valueStart->type == BARANIUM_SOURCE_TOKEN_TYPE_DOT && dotPart->type == BARANIUM_SOURCE_TOKEN_TYPE_NUMBER)
        {
            size_t oldlen = strlen(dotPart->contents);
            dotPart->contents = realloc(dotPart->contents, strlen(dotPart->contents) + 3);
            dotPart->contents[0] = '0';
            dotPart->contents[1] = '.';
            dotPart->contents[oldlen+2] = '0';
            for (size_t i = 0; i < oldlen; i++)
                dotPart->contents[i+2] = dotPart->contents[i];

            return dotPart->contents;
        }

        LOGERROR("Line %d: Invalid float assignment", valueStart->line_number);
    }

    if (varType == BARANIUM_VARIABLE_TYPE_BOOL)
    {
        if (tokens->count > 1)
        {
            LOGERROR("Line %d: Invalid bool assignment: Too many initializing values", baranium_source_token_list_get(tokens, 0)->line_number);
            return NULL;
        }

        baranium_source_token* value = baranium_source_token_list_get(tokens, 0);
        if (value->special_index < BARANIUM_KEYWORD_INDEX_TRUE || value->special_index > BARANIUM_KEYWORD_INDEX_FALSE)
        {
            LOGERROR("Line %d: Invalid bool assignment: Invalid assignment value", value->line_number);
            return NULL;
        }

        return value->contents;
    }

    if (varType == BARANIUM_VARIABLE_TYPE_UINT32 ||
        varType == BARANIUM_VARIABLE_TYPE_INT32)
    {
        if (tokens->count > 2)
        {
            LOGERROR("Line %d: Invalid Int assignment: Too many initializing values", baranium_source_token_list_get(tokens, 0)->line_number);
            return NULL;
        }

        if (tokens->count == 2)
        {
            baranium_source_token* signToken = baranium_source_token_list_get(tokens, 0);
            baranium_source_token* numberToken = baranium_source_token_list_get(tokens, 1);

            if ((signToken->type != BARANIUM_SOURCE_TOKEN_TYPE_PLUS && signToken->type != BARANIUM_SOURCE_TOKEN_TYPE_MINUS) ||
                numberToken->type != BARANIUM_SOURCE_TOKEN_TYPE_NUMBER)
            {
                LOGERROR("Line %d: Invalid Int assignment: Invalid value", signToken->line_number);
                return NULL;
            }

            size_t oldSize = strlen(numberToken->contents);
            for (size_t i = 1; i <= oldSize; i++)
                numberToken->contents[i] = numberToken->contents[i-1];
            numberToken->contents[0] = signToken->contents[0];
            numberToken->contents = realloc(numberToken->contents, oldSize+2);
            numberToken->contents[oldSize+1] = 0;

            return numberToken->contents;
        }

        baranium_source_token* numberToken = baranium_source_token_list_get(tokens, 0);

        if (numberToken->type == BARANIUM_SOURCE_TOKEN_TYPE_NUMBER)
            return numberToken->contents;
        
        if (numberToken->type == BARANIUM_SOURCE_TOKEN_TYPE_NULL)
            return "0";

        LOGERROR("Line %d: Invalid Int assignment: Not a number", numberToken->line_number);
    }

    return "null";
}

uint8_t baranium_token_parser_parse_content_using_depth(size_t* index, baranium_source_token_type_t startType, baranium_source_token_type_t endType, baranium_source_token_list* tokens, baranium_source_token_list* output)
{
    baranium_source_token* nextToken = NULL;
    int depth = 1;
    while (depth > 0 && (*index)+1 < tokens->count)
    {
        (*index)++;
        nextToken = baranium_source_token_list_get(tokens, *index);

        if (nextToken->type == startType)
            depth++;

        if (nextToken->type == endType)
            depth--;

        baranium_source_token_list_add(output, nextToken);
    }

    // to remove the last pushed closing type token
    if (nextToken->type == endType)
        baranium_source_token_list_pop_token(output);

    return depth == 0;
}

void baranium_token_parser_parse_do_while_loop(baranium_loop_token* loop, size_t* index, baranium_source_token* current, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens)
{
    baranium_source_token* nextToken = baranium_source_token_list_get(tokens, *index);
    if (nextToken->type != BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETOPEN)
    {
        LOGERROR("Line %d: Invalid do-while loop, unexpected literal '%s' expected '{'", nextToken->line_number, nextToken->contents);
        return;
    }

    if (!baranium_token_parser_parse_content_using_depth(index, BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETOPEN, BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETCLOSE, tokens, &loop->inner_tokens))
    {
        LOGERROR("Line %d: Invalid contents for do-while loop contents, invalid content depth", current->line_number);
    }
    (*index)++;

    nextToken = baranium_source_token_list_get(tokens, *index);
    if (nextToken->special_index != BARANIUM_KEYWORD_INDEX_WHILE)
    {
        LOGERROR("Line %d: Invalid do-while loop, unexpected literal '%s' expected 'while'", nextToken->line_number, nextToken->contents);
        return;
    }
    (*index)++;

    nextToken = baranium_source_token_list_get(tokens, *index);
    if (nextToken->type != BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN)
    {
        LOGERROR("Line %d: Invalid do-while loop, unexpected literal '%s' expected '('", nextToken->line_number, nextToken->contents);
        return;
    }

    if (!baranium_token_parser_parse_content_using_depth(index, BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN, BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISCLOSE, tokens, &loop->condition.inner_tokens))
    {
        LOGERROR("Line %d: Invalid do-while loop, invalid depth at condition declaration", nextToken->line_number);
        return;
    }
    (*index)++;

    nextToken = baranium_source_token_list_get(tokens, *index);
    if (nextToken->type != BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON)
    {
        LOGERROR("Line %d: Invalid do-while loop, expected ';'", nextToken->line_number);
        return;
    }

    baranium_expression_token_identify(&loop->condition, output, global_tokens);
    baranium_loop_token_parse(loop, output, global_tokens);
}

void baranium_token_parser_parse_for_loop(baranium_loop_token* loop, size_t* index, baranium_source_token* current, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens)
{
    baranium_source_token_list loopVarTokens;
    baranium_source_token_list loopInitializerTokens;
    baranium_source_token_list_init(&loopInitializerTokens);
    baranium_source_token_list_init(&loopVarTokens);

    baranium_source_token* nextToken = baranium_source_token_list_get(tokens, *index);
    if (nextToken->type != BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN)
    {
        LOGERROR("Line %d: Invalid for loop, unexpected literal '%s' expected '('", nextToken->line_number, nextToken->contents);
        return;
    }

    if (!baranium_token_parser_parse_content_using_depth(index, BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN, BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISCLOSE, tokens, &loopInitializerTokens))
    {
        LOGERROR("Line %d: Invalid for loop, invalid depth at declaration", nextToken->line_number);
        return;
    }
    (*index)++;

    nextToken = baranium_source_token_list_get(tokens, *index);
    if (nextToken->type == BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETOPEN)
    {
        if (!baranium_token_parser_parse_content_using_depth(index, BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETOPEN, BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETCLOSE, tokens, &loop->inner_tokens))
        {
            LOGERROR("Line %d: Invalid contents for for loop contents, invalid content depth", current->line_number);
            return;
        }

        goto parseLoopInitializers;
    }
    else if (nextToken->type == BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON)
        goto parseLoopInitializers;

    if (!baranium_token_parser_parse_content_using_depth(index, BARANIUM_SOURCE_TOKEN_TYPE_INVALID, BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON, tokens, &loop->inner_tokens))
    {
        LOGERROR("Line %d: Invalid for loop, unexpected depth", nextToken->line_number);
        return;
    }

parseLoopInitializers:

    if (loopInitializerTokens.count == 0)
    {
        LOGERROR("Line %d: Invalid for loop, unexpected depth", nextToken->line_number);
        return;
    }

    size_t subIndex = 0;
    if (baranium_source_token_list_get(&loopInitializerTokens, subIndex)->type != BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON)
    {
        // because the "read content using depth" function increases the value
        // by 1 once the function starts, we have to decrease our index so
        // that it also adds the first token
        subIndex--;
        baranium_token_parser_parse_content_using_depth(&subIndex, BARANIUM_SOURCE_TOKEN_TYPE_INVALID, BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON, &loopInitializerTokens, &loopVarTokens);

        // because of the `ReadVariable` function we have to add a semicolon at the end of the tokens
        baranium_source_token finalizerToken;
        finalizerToken.contents = ";";
        finalizerToken.special_index = baranium_is_special_char(';');
        finalizerToken.line_number = current->line_number;
        finalizerToken.type = BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON;

        baranium_source_token_list_add(&loopVarTokens, &finalizerToken);
    }
    subIndex++;
    if (baranium_source_token_list_get(&loopInitializerTokens, subIndex)->type != BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON)
    {
        // because the "read content using depth" function increases the value
        // by 1 once the function starts, we have to decrease our index so
        // that it also adds the first token
        subIndex--;
        baranium_token_parser_parse_content_using_depth(&subIndex, BARANIUM_SOURCE_TOKEN_TYPE_INVALID, BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON, &loopInitializerTokens, &loop->condition.inner_tokens);
    }
    subIndex++;
    if (baranium_source_token_list_get(&loopInitializerTokens, subIndex)->type != BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON)
    {
        // because the "read content using depth" function increases the value
        // by 1 once the function starts, we have to decrease our index so
        // that it also adds the first token
        subIndex--;
        baranium_token_parser_parse_content_using_depth(&subIndex, BARANIUM_SOURCE_TOKEN_TYPE_INVALID, BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON, &loopInitializerTokens, &loop->iteration.inner_tokens);
    }

    if (loopVarTokens.count != 0)
    {
        size_t outputSize = output->count;
        subIndex = 0;
        baranium_source_token* varToken = baranium_source_token_list_get(&loopVarTokens, subIndex);
        uint8_t isVar = baranium_is_internal_type(*varToken);
        if (isVar)
            baranium_token_parser_parse_variable(&subIndex, varToken, &loopVarTokens, output, global_tokens);
        else
            baranium_token_parser_parse_expression(&subIndex, varToken, &loopVarTokens, output, global_tokens);

        if (outputSize+1 == output->count)
        {
            if (isVar)
                loop->start_variable = *(baranium_variable_token*)(&output->data[outputSize]);
            else
                loop->start_expression = *(baranium_expression_token*)(&output->data[outputSize]);

            baranium_token_list_remove(output, output->data[output->count-1]->id);
        }
    }

    baranium_loop_token_parse(loop, output, global_tokens);
}

void baranium_token_parser_parse_while_loop(baranium_loop_token* loop, size_t* index, baranium_source_token* current, baranium_source_token_list* tokens, baranium_token_list* output, baranium_token_list* global_tokens)
{
    baranium_source_token* nextToken = baranium_source_token_list_get(tokens, *index);
    if (nextToken->type != BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN)
    {
        LOGERROR("Line %d: Invalid while loop, unexpected literal '%s' expected '('", nextToken->line_number, nextToken->contents);
        return;
    }

    if (!baranium_token_parser_parse_content_using_depth(index, BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN, BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISCLOSE, tokens, &loop->condition.inner_tokens))
    {
        LOGERROR("Line %d: Invalid while loop, invalid depth at declaration", nextToken->line_number);
        return;
    }
    (*index)++;

    nextToken = baranium_source_token_list_get(tokens, *index);
    if (nextToken->type == BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETOPEN)
    {
        if (!baranium_token_parser_parse_content_using_depth(index, BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETOPEN, BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETCLOSE, tokens, &loop->inner_tokens))
        {
            LOGERROR("Line %d: Invalid contents for while loop contents, invalid content depth", current->line_number);
            return;
        }

        goto end;
    }
    else if (nextToken->type == BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON)
        goto end;

    if (!baranium_token_parser_parse_content_using_depth(index, BARANIUM_SOURCE_TOKEN_TYPE_INVALID, BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON, tokens, &loop->inner_tokens))
    {
        LOGERROR("Line %d: Invalid while loop, unexpected depth", nextToken->line_number);
        return;
    }

end:
    baranium_loop_token_parse(loop, output, global_tokens);
}
