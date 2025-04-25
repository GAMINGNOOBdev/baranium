#ifndef __BARANIUM__COMPILER__SOURCE_TOKEN_H_
#define __BARANIUM__COMPILER__SOURCE_TOKEN_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/defines.h>
#include <stdint.h>

#define BARANIUM_SOURCE_TOKEN_LIST_BUFFER_SIZE 0x20

typedef uint8_t baranium_source_token_type_t;

#define BARANIUM_SOURCE_TOKEN_TYPE_INVALID              (baranium_source_token_type_t)-1
#define BARANIUM_SOURCE_TOKEN_TYPE_NUMBER               (baranium_source_token_type_t)0
#define BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD              (baranium_source_token_type_t)1
#define BARANIUM_SOURCE_TOKEN_TYPE_TEXT                 (baranium_source_token_type_t)2
#define BARANIUM_SOURCE_TOKEN_TYPE_NULL                 (baranium_source_token_type_t)3
#define BARANIUM_SOURCE_TOKEN_TYPE_FIELD                (baranium_source_token_type_t)4
#define BARANIUM_SOURCE_TOKEN_TYPE_DOT                  (baranium_source_token_type_t)5
#define BARANIUM_SOURCE_TOKEN_TYPE_COLON                (baranium_source_token_type_t)6
#define BARANIUM_SOURCE_TOKEN_TYPE_COMMA                (baranium_source_token_type_t)7
#define BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN      (baranium_source_token_type_t)8
#define BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISCLOSE     (baranium_source_token_type_t)9
#define BARANIUM_SOURCE_TOKEN_TYPE_BRACKETOPEN          (baranium_source_token_type_t)10
#define BARANIUM_SOURCE_TOKEN_TYPE_BRACKETCLOSE         (baranium_source_token_type_t)11
#define BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETOPEN     (baranium_source_token_type_t)12
#define BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETCLOSE    (baranium_source_token_type_t)13
#define BARANIUM_SOURCE_TOKEN_TYPE_EXCLAMATIONPOINT     (baranium_source_token_type_t)14
#define BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON            (baranium_source_token_type_t)15
#define BARANIUM_SOURCE_TOKEN_TYPE_PLUS                 (baranium_source_token_type_t)16
#define BARANIUM_SOURCE_TOKEN_TYPE_MINUS                (baranium_source_token_type_t)17
#define BARANIUM_SOURCE_TOKEN_TYPE_ASTERISK             (baranium_source_token_type_t)18
#define BARANIUM_SOURCE_TOKEN_TYPE_SLASH                (baranium_source_token_type_t)19
#define BARANIUM_SOURCE_TOKEN_TYPE_MODULO               (baranium_source_token_type_t)20
#define BARANIUM_SOURCE_TOKEN_TYPE_AND                  (baranium_source_token_type_t)21
#define BARANIUM_SOURCE_TOKEN_TYPE_OR                   (baranium_source_token_type_t)22
#define BARANIUM_SOURCE_TOKEN_TYPE_TILDE                (baranium_source_token_type_t)23
#define BARANIUM_SOURCE_TOKEN_TYPE_CARET                (baranium_source_token_type_t)24 // "^"
#define BARANIUM_SOURCE_TOKEN_TYPE_EQUALSIGN            (baranium_source_token_type_t)25
#define BARANIUM_SOURCE_TOKEN_TYPE_EQUALTO              (baranium_source_token_type_t)26
#define BARANIUM_SOURCE_TOKEN_TYPE_NOTEQUAL             (baranium_source_token_type_t)27
#define BARANIUM_SOURCE_TOKEN_TYPE_LESSEQUAL            (baranium_source_token_type_t)28
#define BARANIUM_SOURCE_TOKEN_TYPE_GREATEREQUAL         (baranium_source_token_type_t)29
#define BARANIUM_SOURCE_TOKEN_TYPE_MODEQUAL             (baranium_source_token_type_t)30
#define BARANIUM_SOURCE_TOKEN_TYPE_DIVEQUAL             (baranium_source_token_type_t)31
#define BARANIUM_SOURCE_TOKEN_TYPE_MULEQUAL             (baranium_source_token_type_t)32
#define BARANIUM_SOURCE_TOKEN_TYPE_MINUSEQUAL           (baranium_source_token_type_t)33
#define BARANIUM_SOURCE_TOKEN_TYPE_PLUSEQUAL            (baranium_source_token_type_t)34
#define BARANIUM_SOURCE_TOKEN_TYPE_ANDEQUAL             (baranium_source_token_type_t)35
#define BARANIUM_SOURCE_TOKEN_TYPE_OREQUAL              (baranium_source_token_type_t)36
#define BARANIUM_SOURCE_TOKEN_TYPE_XOREQUAL             (baranium_source_token_type_t)37
#define BARANIUM_SOURCE_TOKEN_TYPE_ANDAND               (baranium_source_token_type_t)38
#define BARANIUM_SOURCE_TOKEN_TYPE_OROR                 (baranium_source_token_type_t)39
#define BARANIUM_SOURCE_TOKEN_TYPE_MINUSMINUS           (baranium_source_token_type_t)40
#define BARANIUM_SOURCE_TOKEN_TYPE_PLUSPLUS             (baranium_source_token_type_t)41
#define BARANIUM_SOURCE_TOKEN_TYPE_BITSHIFTLEFT         (baranium_source_token_type_t)42
#define BARANIUM_SOURCE_TOKEN_TYPE_BITSHIFTRIGHT        (baranium_source_token_type_t)43
#define BARANIUM_SOURCE_TOKEN_TYPE_QUOTE                (baranium_source_token_type_t)44
#define BARANIUM_SOURCE_TOKEN_TYPE_DOUBLEQUOTE          (baranium_source_token_type_t)45
#define BARANIUM_SOURCE_TOKEN_TYPE_LESSTHAN             (baranium_source_token_type_t)46
#define BARANIUM_SOURCE_TOKEN_TYPE_GREATERTHAN          (baranium_source_token_type_t)47

typedef struct baranium_source_token
{
    baranium_source_token_type_t type;
    char* contents;
    int special_index;
    int line_number;
} baranium_source_token;

/**
 * @brief Get the string representation of a token type
 * 
 * @param type The token type
 * @return The string representation of a token type
 */
BARANIUMAPI const char* baranium_source_token_type_to_string(baranium_source_token_type_t type);

typedef struct baranium_source_token_list
{
    baranium_source_token* data;
    baranium_source_token* current;
    size_t buffer_size;
    size_t index;
    size_t count;
} baranium_source_token_list;

// forward declaration
struct baranium_compiler_context;

// create and initialize a source token list
BARANIUMAPI void baranium_source_token_list_init(baranium_source_token_list* obj);

// dispose a source token list
BARANIUMAPI void baranium_source_token_list_dispose(baranium_source_token_list* obj);

// clear a source token list
BARANIUMAPI void baranium_source_token_list_clear(baranium_source_token_list* obj);

// reset the iteration progress
BARANIUMAPI void baranium_source_token_list_reset(baranium_source_token_list* obj);

// insert another list into the list after a certain point
BARANIUMAPI void baranium_source_token_list_insert_after(baranium_source_token_list* obj, int index, baranium_source_token_list* other);

// check if we reached the end of the list
BARANIUMAPI uint8_t baranium_source_token_list_end_of_list(baranium_source_token_list* obj);

// allocate/create a source token entry
BARANIUMAPI void baranium_source_token_list_add(baranium_source_token_list* obj, baranium_source_token* data);

// delete and free memory used by a source token entry
BARANIUMAPI void baranium_source_token_list_remove(baranium_source_token_list* obj, size_t index);

// get a source token if existent
BARANIUMAPI baranium_source_token* baranium_source_token_list_get(baranium_source_token_list* obj, size_t index);

// push a list of tokens to the end of the list (and clears the `other` list)
BARANIUMAPI void baranium_source_token_list_push_list(baranium_source_token_list* obj, baranium_source_token_list* other);

// pop the last added token from the list
BARANIUMAPI void baranium_source_token_list_pop_token(baranium_source_token_list* obj);

// check if the next tokens type matches the desired type
BARANIUMAPI uint8_t baranium_source_token_list_next_matches(baranium_source_token_list* obj, baranium_source_token_type_t type);

// get the current token
BARANIUMAPI baranium_source_token* baranium_source_token_list_current(baranium_source_token_list* obj);

// go to the next token and consume the last one
BARANIUMAPI baranium_source_token* baranium_source_token_list_next(baranium_source_token_list* obj);

// get the next token without consuming the current one
BARANIUMAPI baranium_source_token* baranium_source_token_list_peek(baranium_source_token_list* obj);

#ifdef __cplusplus
}
#endif

#endif
