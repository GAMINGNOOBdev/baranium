#ifndef __BARANIUM__STRING_UTIL_H_
#define __BARANIUM__STRING_UTIL_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/defines.h>
#include <stdint.h>
#include <stddef.h>

#define BARANIUM_STRING_LIST_BUFFER_SIZE 0x80 // should be an okay initial size, realloc should just handle it fine
#define BARANIUM_STRING_MAP_BUFFER_SIZE BARANIUM_STRING_LIST_BUFFER_SIZE

///////////////////
///             ///
/// STRING LIST ///
///             ///
///////////////////

typedef struct baranium_string_list
{
    size_t count;
    size_t buffer_size;

    index_t* hashes;
    const char** strings;
} baranium_string_list;

/**
 * @brief Initialize a string list
*/
BARANIUMAPI void baranium_string_list_init(baranium_string_list* list);

/**
 * @brief Get the index of a string inside the list using it's hash
*/
BARANIUMAPI int baranium_string_list_get_index(baranium_string_list* list, index_t hash);

/**
 * @brief Get the index of a string inside the list
*/
BARANIUMAPI int baranium_string_list_get_index_of_string(baranium_string_list* list, const char* string);

/**
 * @brief Add an item to the string list
*/
BARANIUMAPI void baranium_string_list_add(baranium_string_list* list, const char* string);

/**
 * @brief Remove the last item from the string list
*/
BARANIUMAPI void baranium_string_list_remove_last(baranium_string_list* list);

/**
 * @brief Get a string from the list using an index
*/
BARANIUMAPI const char* baranium_string_list_get_string(baranium_string_list* list, int index);

/**
 * @brief Get a string from the list using a hash
*/
BARANIUMAPI const char* baranium_string_list_get_string_from_hash(baranium_string_list* list, index_t hash);

/**
 * @brief Dispose a string list
*/
BARANIUMAPI void baranium_string_list_dispose(baranium_string_list* list);

//////////////////////
///                ///
/// STRING HASHMAP ///
///                ///
//////////////////////

typedef struct baranium_string_map
{
    size_t count;
    size_t buffer_size;

    index_t* hashes;
    const char** strings;
} baranium_string_map;

/**
 * @brief Initialize a string map
*/
BARANIUMAPI void baranium_string_map_init(baranium_string_map* map);

/**
 * @brief Get the index of a string inside the map
*/
BARANIUMAPI int baranium_string_map_get_index(baranium_string_map* map, const char* name);

/**
 * @brief Get the index of a string inside the map using it's hash
*/
BARANIUMAPI int baranium_string_map_get_index_of_hash(baranium_string_map* map, index_t hash);

/**
 * @brief Add an item to the string map
*/
BARANIUMAPI void baranium_string_map_add(baranium_string_map* map, const char* name, const char* string);

/**
 * @brief Remove a given item from the string map
*/
BARANIUMAPI void baranium_string_map_remove(baranium_string_map* map, const char* name);

/**
 * @brief Get a string from the map
*/
BARANIUMAPI const char* baranium_string_map_get(baranium_string_map* map, const char* name);

/**
 * @brief Get a string from the map using it's hash
*/
BARANIUMAPI const char* baranium_string_map_get_from_hash(baranium_string_map* map, index_t hash);

/**
 * @brief Dispose a string map
*/
BARANIUMAPI void baranium_string_map_dispose(baranium_string_map* map);

//////////////////////////////////////
///                                ///
/// OTHER STRING RELATED FUNCTIONS ///
///                                ///
//////////////////////////////////////

/**
 * @brief Get the  index of a specific `delim` inside `str`(-1 if not found)
*/ 
BARANIUMAPI int stridx(const char* string, char delim);

/**
 * @brief Get the  index of a specific char that is NOT `delim` inside `str`(-1 if not found)
*/ 
BARANIUMAPI int stridxnot(const char* string, char delim);

/**
 * @brief Get the last index of a specific `delim` inside `str`(-1 if not found)
*/ 
BARANIUMAPI int stridxlast(const char* string, char delim);

/**
 * @brief Get the last index of a specific char that is NOT `delim` inside `str`(-1 if not found)
*/ 
BARANIUMAPI int stridxnotlast(const char* string, char delim);

/**
 * @brief Get the number of how many times a specific char `delim` is inside `str`
*/
BARANIUMAPI int strdelimcount(const char* string, char delim);

/**
 * @brief Trims leading whitespaces at the front and back
 *
 * @note This will `free` the src pointer!
*/
BARANIUMAPI char* strtrimleading(const char* src);

/**
 * @brief Trims any comment part of a string
 *
 * @note This will `free` the src pointer!
*/
BARANIUMAPI char* strtrimcomment(const char* src);

/**
 * @brief Create a substring from `start` with a specific `length` (pass -1 to get everything till the end)
*/
BARANIUMAPI char* strsubstr(const char* src, size_t start, size_t length);

/**
 * @brief Check if a string is a number
*/
BARANIUMAPI uint8_t strisnum(const char* src);

/**
 * @brief Get the floating point number value from a string
*/
BARANIUMAPI float strgetfloatval(const char* str);

/**
 * @brief Get the number value from a string
*/
BARANIUMAPI uint64_t strgetnumval(const char* src);

/**
 * @brief Connects escape sequences written out in plain text into actual escape sequence characters
 *
 * @note This will `free` the src pointer!
*/
BARANIUMAPI char* strconescseq(const char* src);

/**
 * @brief Split a string when coming across `delim`
*/
BARANIUMAPI void strsplit(baranium_string_list* _out, const char* src, char delim);

#ifdef __cplusplus
}
#endif

#endif
