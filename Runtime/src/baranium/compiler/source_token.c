#include <baranium/compiler/source_token.h>
#include <stdlib.h>
#include <memory.h>

const char* baranium_source_token_type_to_string(baranium_source_token_type_t type)
{
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_INVALID)
        return "INVALID";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_NUMBER)
        return "NUMBER";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD)
        return "KEYWORD";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_TEXT)
        return "TEXT";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_NULL)
        return "NULL";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_FIELD)
        return "FIELD";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_DOT)
        return "DOT";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_COLON)
        return "COLON";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_COMMA)
        return "COMMA";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN)
        return "PARENTHESISOPEN";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISCLOSE)
        return "PARENTHESISCLOSE";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_BRACKETOPEN)
        return "BRACKETOPEN";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_BRACKETCLOSE)
        return "BRACKETCLOSE";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETOPEN)
        return "CURLYBRACKETOPEN";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETCLOSE)
        return "CURLYBRACKETCLOSE";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_EXCLAMATIONPOINT)
        return "EXCLAMATIONPOINT";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON)
        return "SEMICOLON";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_PLUS)
        return "PLUS";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_MINUS)
        return "MINUS";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_ASTERISK)
        return "ASTERISK";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_SLASH)
        return "SLASH";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_MODULO)
        return "MODULO";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_AND)
        return "AND";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_OR)
        return "OR";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_TILDE)
        return "TILDE";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_CARET)
        return "CARET";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_EQUALSIGN)
        return "EQUALSIGN";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_EQUALTO)
        return "EQUALTO";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_NOTEQUAL)
        return "NOTEQUAL";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_LESSEQUAL)
        return "LESSEQUAL";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_GREATEREQUAL)
        return "GREATEREQUAL";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_MODEQUAL)
        return "MODEQUAL";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_DIVEQUAL)
        return "DIVEQUAL";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_MULEQUAL)
        return "MULEQUAL";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_MINUSEQUAL)
        return "MINUSEQUAL";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_PLUSEQUAL)
        return "PLUSEQUAL";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_ANDEQUAL)
        return "ANDEQUAL";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_OREQUAL)
        return "OREQUAL";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_XOREQUAL)
        return "XOREQUAL";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_ANDAND)
        return "ANDAND";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_OROR)
        return "OROR";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_MINUSMINUS)
        return "MINUSMINUS";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_PLUSPLUS)
        return "PLUSPLUS";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_BITSHIFTLEFT)
        return "BITSHIFTLEFT";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_BITSHIFTRIGHT)
        return "BITSHIFTRIGHT";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_QUOTE)
        return "QUOTE";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_DOUBLEQUOTE)
        return "DOUBLEQUOTE";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_LESSTHAN)
        return "LESSTHAN";
    if (type == BARANIUM_SOURCE_TOKEN_TYPE_GREATERTHAN)
        return "GREATERTHAN";

    return "none";
}

void baranium_source_token_list_init(baranium_source_token_list* obj)
{
    if (obj == NULL) return;

    memset(obj, 0, sizeof(baranium_source_token_list));
}

void baranium_source_token_list_dispose(baranium_source_token_list* obj)
{
    if (obj == NULL) return;

    if (obj->data != NULL)
        free(obj->data);

    memset(obj, 0, sizeof(baranium_source_token_list));
}

void baranium_source_token_list_clear(baranium_source_token_list* obj)
{
    if (obj == NULL) return;

    baranium_source_token_list_dispose(obj);
}

void baranium_source_token_list_reset(baranium_source_token_list* obj)
{
    if (obj == NULL) return;
    obj->current = obj->data;
    obj->index = 0;
}

void baranium_source_token_list_insert_after(baranium_source_token_list* obj, int index, baranium_source_token_list* other)
{
    if (obj == NULL || other == NULL || other->data == NULL)
        return;

    size_t free_space = obj->buffer_size - obj->count;
    if (free_space < other->count)
    {
        obj->buffer_size += other->buffer_size;
        obj->data = realloc(obj->data, sizeof(baranium_source_token)*obj->buffer_size);
    }

    memmove(&obj->data[index+other->count+1], &obj->data[index+1], sizeof(baranium_source_token)*(obj->count-index-1));
    memcpy(&obj->data[index+1], other->data, sizeof(baranium_source_token)*other->count);
    obj->count += other->count;
}

uint8_t baranium_source_token_list_end_of_list(baranium_source_token_list* obj)
{
    if (obj == NULL) return 1;

    return obj->index > obj->count;
}

void baranium_source_token_list_add(baranium_source_token_list* obj, baranium_source_token* data)
{
    if (obj == NULL || data == NULL || data->contents == NULL)
        return;

    if (obj->buffer_size <= obj->count+1)
    {
        obj->buffer_size += BARANIUM_SOURCE_TOKEN_LIST_BUFFER_SIZE;
        obj->data = realloc(obj->data, sizeof(baranium_source_token)*obj->buffer_size);
    }

    memcpy(&obj->data[obj->count], data, sizeof(baranium_source_token));
    obj->count++;
}

void baranium_source_token_list_remove(baranium_source_token_list* obj, size_t index)
{
    if (obj == NULL || index == (size_t)-1) return;
    if (obj->data == NULL || obj->count <= index) return;
    if (obj->count == 0) return;

    if (index == obj->count-1)
    {
        memset(&obj->data[index], 0, sizeof(baranium_source_token));
        obj->count--;
        return;
    }

    memset(&obj->data[index], 0, sizeof(baranium_source_token));
    memmove(&obj->data[index], &obj->data[index+1], sizeof(baranium_source_token)*(obj->count - index - 1));
    obj->count--;
}

baranium_source_token* baranium_source_token_list_get(baranium_source_token_list* obj, size_t index)
{
    if (obj == NULL || index == (size_t)-1) return NULL;
    if (obj->data == NULL || obj->count <= index) return NULL;

    return &obj->data[index];
}

void baranium_source_token_list_push_list(baranium_source_token_list* obj, baranium_source_token_list* other)
{
    if (obj == NULL || other == NULL) return;
    if (other->data == NULL) return;

    if (obj->data == NULL)
    {
        obj->buffer_size = other->buffer_size;
        obj->data = malloc(sizeof(baranium_source_token)*obj->buffer_size);
    }

    size_t free_space = obj->buffer_size - obj->count;
    if (free_space < other->count)
    {
        obj->buffer_size += other->buffer_size;
        obj->data = realloc(obj->data, sizeof(baranium_source_token)*obj->buffer_size);
    }

    memcpy(&obj->data[obj->count], other->data, sizeof(baranium_source_token)*other->count);
    obj->count += other->count;

    free(other->data);
    memset(other, 0, sizeof(baranium_source_token_list));   
}

void baranium_source_token_list_pop_token(baranium_source_token_list* obj)
{
    if (obj == NULL) return;
    baranium_source_token_list_remove(obj, obj->count-1);
}

uint8_t baranium_source_token_list_next_matches(baranium_source_token_list* obj, baranium_source_token_type_t type)
{
    baranium_source_token* token = baranium_source_token_list_peek(obj);
    if (token == NULL)
        return 0;

    if (token->type != type)
        return 0;

    baranium_source_token_list_next(obj);
    return 1;
}

baranium_source_token* baranium_source_token_list_current(baranium_source_token_list* obj)
{
    if (obj == NULL) return NULL;
    if (baranium_source_token_list_end_of_list(obj)) return NULL;

    if (obj->index == 0)
        return baranium_source_token_list_get(obj, 0);

    return baranium_source_token_list_get(obj, obj->index-1);
}

baranium_source_token* baranium_source_token_list_next(baranium_source_token_list* obj)
{
    if (obj == NULL) return NULL;
    if (obj->index+1 > obj->count) return NULL;

    baranium_source_token* token = baranium_source_token_list_get(obj, obj->index);
    obj->index++;
    return token;
}

baranium_source_token* baranium_source_token_list_peek(baranium_source_token_list* obj)
{
    if (obj == NULL) return NULL;
    if (baranium_source_token_list_end_of_list(obj)) return NULL;

    return baranium_source_token_list_get(obj, obj->index);
}
