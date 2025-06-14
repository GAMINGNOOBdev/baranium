/**
 * @note THIS IS NOT INTENDED FOR USE BY THE USER OF THE RUNTIME!
 *       This header is intended to be used internally by the runtime
 *       and therefore, functions defined in this header cannot be used
 *       by the user.
 */
#ifndef __BARANIUM__COMPILER__LANGUAGE__LANGUAGE_H_
#define __BARANIUM__COMPILER__LANGUAGE__LANGUAGE_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/compiler/source_token.h>

#define BARANIUM_KEYWORDS_TYPES_START 3
#define BARANIUM_KEYWORDS_TYPES_END 16

#define BARANIUM_KEYWORD_INDEX_FIELD         0
#define BARANIUM_KEYWORD_INDEX_ATTACHED      1
#define BARANIUM_KEYWORD_INDEX_NULL          2
#define BARANIUM_KEYWORD_INDEX_OBJECT        3
#define BARANIUM_KEYWORD_INDEX_STRING        4
#define BARANIUM_KEYWORD_INDEX_FLOAT         5
#define BARANIUM_KEYWORD_INDEX_BOOL          6
#define BARANIUM_KEYWORD_INDEX_INT32         7
#define BARANIUM_KEYWORD_INDEX_UINT32        8
#define BARANIUM_KEYWORD_INDEX_DOUBLE        9
#define BARANIUM_KEYWORD_INDEX_INT8          10
#define BARANIUM_KEYWORD_INDEX_UINT8         11
#define BARANIUM_KEYWORD_INDEX_INT16         12
#define BARANIUM_KEYWORD_INDEX_UINT16        13
#define BARANIUM_KEYWORD_INDEX_INT64         14
#define BARANIUM_KEYWORD_INDEX_UINT64        15
#define BARANIUM_KEYWORD_INDEX_VOID          16
#define BARANIUM_KEYWORD_INDEX_DEFINE        17
#define BARANIUM_KEYWORD_INDEX_RETURN        18
#define BARANIUM_KEYWORD_INDEX_TRUE          19
#define BARANIUM_KEYWORD_INDEX_FALSE         20
#define BARANIUM_KEYWORD_INDEX_IF            21
#define BARANIUM_KEYWORD_INDEX_ELSE          22
#define BARANIUM_KEYWORD_INDEX_INSTANTIATE   23
#define BARANIUM_KEYWORD_INDEX_DELETE        24
#define BARANIUM_KEYWORD_INDEX_ATTACH        25
#define BARANIUM_KEYWORD_INDEX_DETACH        26
#define BARANIUM_KEYWORD_INDEX_DO            27
#define BARANIUM_KEYWORD_INDEX_FOR           28
#define BARANIUM_KEYWORD_INDEX_WHILE         29
#define BARANIUM_KEYWORD_INDEX_BREAK         30
#define BARANIUM_KEYWORD_INDEX_CONTINUE      31

typedef struct baranium_keyword
{
    const char* name;
    baranium_source_token_type_t type;
} baranium_keyword;

typedef struct baranium_special_operator
{
    const char* name;
    baranium_source_token_type_t type;
} baranium_special_operator;

typedef struct baranium_special_character
{
    const char* name;
    baranium_source_token_type_t type;
} baranium_special_character;

extern baranium_keyword baranium_keywords[];
extern baranium_special_operator baranium_special_operators[];
extern baranium_special_character baranium_special_characters[];
extern baranium_special_character baranium_special_operation_characters[];

/**
 * @brief Checks if the given token is an internal type
 * 
 * @param token The token that will be checked
 * @return `true` if the token is an internal type
 * @return `false` otherwise
*/
BARANIUMAPI uint8_t baranium_is_internal_type(baranium_source_token token);

/**
 * @brief Checks if the given string is a keyword
 * 
 * @param string The string that will be checked if it is a keyword
 * 
 * @returns -1 if not a keyword, else it will return the index of it
*/
BARANIUMAPI int baranium_is_keyword(const char* string);

/**
 * @brief Checks if the given characters `a` and `b` make up a special operator
 * 
 * @param a The first character of the operator
 * @param b The second character of the operator
 * 
 * @returns -1 if not a special operator, else an index to the special operator in the list
*/
BARANIUMAPI int baranium_is_special_operator(char a, char b);

/**
 * @brief Checks if the given character `c` is a special character
 * 
 * @param c The character that will be checked
 * 
 * @returns -1 if not a special character, else an index to the special character in the list
*/
BARANIUMAPI int baranium_is_special_char(char c);

#ifdef __cplusplus
}
#endif

#endif
