#ifndef __BARANIUM__COMPILER__BINARIES__SYMBOL_TABLE_H_
#define __BARANIUM__COMPILER__BINARIES__SYMBOL_TABLE_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#define BARANIUM_SYMBOL_TABLE_BUFFER_SIZE 0x80

#include <baranium/compiler/language/variable_token.h>
#include <stddef.h>
#include <stdint.h>

typedef int64_t index_t;

/**
 * @brief An entry of a `baranium_symbol_table`
 */
typedef struct
{
    const char* name;
    index_t id;
} baranium_symbol_table_entry;

/**
 * @brief A table containing names and their respective internal ID
 * 
 * @note This is only useful while compilation to check for temporary variables
 */
typedef struct
{
    baranium_symbol_table_entry* data;
    size_t buffer_size;
    size_t count;
} baranium_symbol_table;

/**
 * @brief Initialize a `baranium_symbol_table`
 * 
 * @param table Variable table
 */
void baranium_symbol_table_init(baranium_symbol_table* table);

/**
 * @brief Dispose a `baranium_symbol_table`
 * 
 * @param table Variable table
 */
void baranium_symbol_table_dispose(baranium_symbol_table* table);

/**
 * @brief Clear the variable table
 * 
 * @param table Variable table
 */
void baranium_symbol_table_clear(baranium_symbol_table* table);

/**
 * @brief Look for a specific entry
 * 
 * @param table Variable table
 * @param name Name of the entry
 * @returns The id of the entry
 */
index_t baranium_symbol_table_lookup(baranium_symbol_table* table, const char* name);

/**
 * @brief Look for a specific entry
 * 
 * @param table Variable table
 * @param id ID of the entry
 * @returns The name of the entry
 */
const char* baranium_symbol_table_lookup_name(baranium_symbol_table* table, index_t id);

/**
 * @brief Add a variable entry
 * 
 * @param table Variable table
 * @param var The variable for which the entry will be created
 */
void baranium_symbol_table_add(baranium_symbol_table* table, baranium_variable_token* var);

/**
 * @brief Add a variable entry
 * 
 * @param table Variable table
 * @param name The variable name for which the entry will be created
 * @param id The variable id for which the entry will be created
 */
void baranium_symbol_table_add_from_name_and_id(baranium_symbol_table* table, const char* name, index_t id);

/**
 * @brief Remove a variable entry
 * 
 * @param table Variable table
 * @param var The variable that will be removed
 */
void baranium_symbol_table_remove(baranium_symbol_table* table, baranium_variable_token* var);

/**
 * @brief Copy a variable table
 * 
 * @param table Variable table
 * @param other The other variable table
 */
void baranium_symbol_table_copy(baranium_symbol_table* table, baranium_symbol_table* other);

#ifdef __cplusplus
}
#endif

#endif
