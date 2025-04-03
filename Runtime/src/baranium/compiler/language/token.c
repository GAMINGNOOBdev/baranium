#include "baranium/logging.h"
#include <baranium/compiler/language/expression_token.h>
#include <baranium/compiler/language/function_token.h>
#include <baranium/compiler/language/variable_token.h>
#include <baranium/compiler/language/if_else_token.h>
#include <baranium/compiler/language/field_token.h>
#include <baranium/compiler/language/loop_token.h>
#include <baranium/compiler/language/token.h>
#include <baranium/defines.h>
#include <stdlib.h>
#include <memory.h>

const char* baranium_token_type_to_string(baranium_token_type_t type)
{
    switch (type)
    {
        default:
        case BARANIUM_TOKEN_TYPE_INVALID:
            return "Invalid";

        case BARANIUM_TOKEN_TYPE_FUNCTION:
            return "Function";

        case BARANIUM_TOKEN_TYPE_FIELD:
            return "Field";

        case BARANIUM_TOKEN_TYPE_VARIABLE:
            return "Variable";

        case BARANIUM_TOKEN_TYPE_EXPRESSION:
            return "Expression";

        case BARANIUM_TOKEN_TYPE_IFELSESTATEMENT:
            return "IfElseStatement";

        case BARANIUM_TOKEN_TYPE_DOWHILELOOP:
            return "DoWhileLoop";

        case BARANIUM_TOKEN_TYPE_WHILELOOP:
            return "WhileLoop";

        case BARANIUM_TOKEN_TYPE_FORLOOP:
            return "ForLoop";
    }
}

void baranium_token_list_init(baranium_token_list* obj)
{
    if (obj == NULL) return;

    memset(obj, 0, sizeof(baranium_token_list));
}

void baranium_token_list_dispose(baranium_token_list* obj)
{
    if (obj == NULL) return;

    if (obj->data != NULL)
    {
        LOGDEBUG(stringf("disposing token list 0x%x with '%lld' children", obj->data, obj->count));
        for (size_t i = 0; i < obj->count; i++)
        {
            baranium_token* token = obj->data[i];
            if (token == NULL)
                continue;

            baranium_token_type_t type = token->type;
            LOGDEBUG(stringf("token 0x%x{ .type=%s, .id=%lld }", token, baranium_token_type_to_string(type), token->id));

            if (type == BARANIUM_TOKEN_TYPE_FUNCTION)
                baranium_function_token_dispose((baranium_function_token*)token);
            if (type == BARANIUM_TOKEN_TYPE_DOWHILELOOP || type == BARANIUM_TOKEN_TYPE_WHILELOOP || type == BARANIUM_TOKEN_TYPE_FORLOOP)
                baranium_loop_token_dispose((baranium_loop_token*)token);
            if (type == BARANIUM_TOKEN_TYPE_IFELSESTATEMENT)
                baranium_if_else_token_dispose((baranium_if_else_token*)token);
            if (type == BARANIUM_TOKEN_TYPE_EXPRESSION)
                baranium_expression_token_dispose((baranium_expression_token*)token);
            if (type == BARANIUM_TOKEN_TYPE_FIELD)
                baranium_field_token_dispose((baranium_field_token*)token);
            if (type == BARANIUM_TOKEN_TYPE_VARIABLE)
                baranium_variable_token_dispose((baranium_variable_token*)token);

            free(token);
        }

        if (obj->count > 0)
            free(obj->data);
    }
}

void baranium_token_list_clear(baranium_token_list* obj)
{
    if (obj == NULL) return;
    baranium_token_list_dispose(obj);
    memset(obj, 0, sizeof(baranium_token_list));
}

void baranium_token_list_add(baranium_token_list* obj, baranium_token* data)
{
    if (obj == NULL || data == NULL) return;
    if (data->type == BARANIUM_TOKEN_TYPE_INVALID) return;

    if (obj->buffer_size <= obj->count+1)
    {
        obj->buffer_size += BARANIUM_TOKEN_LIST_BUFFER_SIZE;
        obj->data = realloc(obj->data, sizeof(baranium_token*)*obj->buffer_size);
    }

    obj->data[obj->count] = data;
    obj->count++;
}

int baranium_token_list_get_index(baranium_token_list* obj, index_t id)
{
    if (obj == NULL || id == BARANIUM_INVALID_INDEX) return -1;

    for (size_t i = 0; i < obj->count; i++)
        if (obj->data[i]->id == id)
            return i;

    return -1;
}

void baranium_token_list_remove(baranium_token_list* obj, index_t id)
{
    if (obj == NULL || id == BARANIUM_INVALID_INDEX) return;
    if (obj->count == 0) return;

    int index = baranium_token_list_get_index(obj, id);
    if (index == -1)
        return;

    baranium_token_list_remove_at(obj, index);
}

void baranium_token_list_remove_at(baranium_token_list* obj, size_t index)
{
    baranium_token_list_remove_n_at(obj, index, 1);
}

void baranium_token_list_remove_n_at(baranium_token_list* obj, size_t index, size_t n)
{
    if (obj == NULL || index == (size_t)-1 || n == (size_t)-1 || n == 0) return;
    if (obj->count == 0) return;

    for (size_t i = 0; i < n; i++)
    {
        for (size_t idx = index; idx < obj->count-1; idx++)
        {
            baranium_token* tmp = NULL;
            size_t tmpindex = idx+1;
            if (tmpindex < obj->count)
                tmp = obj->data[tmpindex];
            obj->data[idx] = tmp;
        }
        obj->count--;
    }
}

baranium_token* baranium_token_list_get(baranium_token_list* obj, index_t id)
{
    if (obj == NULL || id == BARANIUM_INVALID_INDEX) return NULL;
    int index = baranium_token_list_get_index(obj, id);
    if (index == -1)
        return NULL;
    return obj->data[index];
}

baranium_token* baranium_token_list_find(baranium_token_list* obj, const char* name)
{
    if (obj == NULL || obj->data == NULL || name == NULL)
        return NULL;

    for (size_t i = 0; i < obj->count; i++)
    {
        if (obj->data[i]->name == NULL)
            continue;

        if (strcmp(obj->data[i]->name, name) == 0)
            return obj->data[i];
    }

    return NULL;
}

baranium_token* baranium_token_lists_contain(const char* name, baranium_token_list* list1, baranium_token_list* list2)
{
    baranium_token* result1 = baranium_token_list_find(list1, name);
    baranium_token* result2 = baranium_token_list_find(list2, name);

    if (result1 != NULL)
        return result1;

    return result2;
}
