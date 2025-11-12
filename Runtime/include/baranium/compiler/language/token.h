/**
 * @note THIS IS NOT INTENDED FOR USE BY THE USER OF THE RUNTIME!
 *       This header is intended to be used internally by the runtime
 *       and therefore, functions defined in this header cannot be used
 *       by the user.
 */
#ifndef __BARANIUM__COMPILER__LANGUAGE__TOKEN_H_
#define __BARANIUM__COMPILER__LANGUAGE__TOKEN_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/defines.h>

#define BARANIUM_TOKEN_TYPE_INVALID             (baranium_token_type_t)-1
#define BARANIUM_TOKEN_TYPE_FIELD               (baranium_token_type_t)0
#define BARANIUM_TOKEN_TYPE_VARIABLE            (baranium_token_type_t)1
#define BARANIUM_TOKEN_TYPE_FUNCTION            (baranium_token_type_t)2
#define BARANIUM_TOKEN_TYPE_EXPRESSION          (baranium_token_type_t)3
#define BARANIUM_TOKEN_TYPE_IFELSESTATEMENT     (baranium_token_type_t)4
#define BARANIUM_TOKEN_TYPE_DOWHILELOOP         (baranium_token_type_t)5
#define BARANIUM_TOKEN_TYPE_WHILELOOP           (baranium_token_type_t)6
#define BARANIUM_TOKEN_TYPE_FORLOOP             (baranium_token_type_t)7
#define BARANIUM_TOKEN_TYPE_STRUCT              (baranium_token_type_t)8

#define BARANIUM_TOKEN_LIST_BUFFER_SIZE 0x80

typedef uint8_t baranium_token_type_t;

/**
 * @brief Get the string representation of a token type
 * 
 * @param type The token type that will be "stringified"
 * 
 * @return The string representation of `type`
*/
BARANIUMAPI const char* baranium_token_type_to_string(baranium_token_type_t type);

typedef struct baranium_token
{
    index_t id;
    const char* name;
    baranium_token_type_t type;
} baranium_token;

typedef struct baranium_token_list
{
    baranium_token** data;
    size_t buffer_size;
    size_t count;
} baranium_token_list;

/**
 * @brief Initialize a token list
*/
BARANIUMAPI void baranium_token_list_init(baranium_token_list* obj);

/**
 * @brief Dispose a token list
*/
BARANIUMAPI void baranium_token_list_dispose(baranium_token_list* obj);

/**
 * @brief Clear a token list
*/
BARANIUMAPI void baranium_token_list_clear(baranium_token_list* obj);

/**
 * @brief Add a token to the token list
*/
BARANIUMAPI void baranium_token_list_add(baranium_token_list* obj, baranium_token* data);

/**
 * @brief Get the index of a token from the list
*/
BARANIUMAPI int baranium_token_list_get_index(baranium_token_list* obj, index_t id);

/**
 * @brief Remove a token from the list
*/
BARANIUMAPI void baranium_token_list_remove(baranium_token_list* obj, index_t id);

/**
 * @brief Remove a token from the list using it's index in the list
*/
BARANIUMAPI void baranium_token_list_remove_at(baranium_token_list* obj, size_t index);

/**
 * @brief Remove `n` tokens from the list using it's index in the list
*/
BARANIUMAPI void baranium_token_list_remove_n_at(baranium_token_list* obj, size_t index, size_t n);

/**
 * @brief Get a token from the list
*/
BARANIUMAPI baranium_token* baranium_token_list_get(baranium_token_list* obj, index_t id);

/**
 * @brief Find a token from the list
*/
BARANIUMAPI baranium_token* baranium_token_list_find(baranium_token_list* obj, const char* name);

/**
 * @brief Find a token from the list
*/
BARANIUMAPI baranium_token* baranium_token_lists_contain(const char* name, baranium_token_list* list1, baranium_token_list* list2);

#ifdef __cplusplus
}
#endif

#endif
