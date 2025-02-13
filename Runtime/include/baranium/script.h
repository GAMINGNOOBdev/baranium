#ifndef __BARANIUM__SCRIPT_H_
#define __BARANIUM__SCRIPT_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "defines.h"
#include "field.h"
#include "variable.h"
#include "function.h"

#define MAGIC_NUM_0 'B'
#define MAGIC_NUM_1 'G'
#define MAGIC_NUM_2 'S'
#define MAGIC_NUM_3 'L'

#define SECTION_TYPE_INVALID     (baranium_section_type_t)0x00
#define SECTION_TYPE_FIELDS      (baranium_section_type_t)0x01
#define SECTION_TYPE_VARIABLES   (baranium_section_type_t)0x02
#define SECTION_TYPE_FUNCTIONS   (baranium_section_type_t)0x03

typedef uint8_t baranium_section_type_t;

typedef struct baranium_section
{
    uint8_t type;
    index_t id;
    uint64_t data_size;
    uint64_t data_location; // mostly used by function sections because code size can sometimes be quite big and code should probably be dynamically loaded and unloaded when not needed
    uint8_t* data;

    struct baranium_section* next;
} baranium_section;

typedef struct
{
    uint8_t magic[4];
    uint32_t version;
    uint64_t section_count;
} baranium_script_header;

typedef struct baranium_script_name_table_entry
{
    uint8_t length;
    uint8_t* name;
    index_t id;

    struct baranium_script_name_table_entry* next;
} baranium_script_name_table_entry;

typedef struct
{
    uint64_t name_count;
    baranium_script_name_table_entry* entries_start;
    baranium_script_name_table_entry* entries_end;
} baranium_script_name_table;

typedef struct baranium_script
{
    baranium_script_header header;
    baranium_section* sections_start;
    baranium_section* sections_end;
    baranium_script_name_table nametable;
    baranium_handle* handle;
} baranium_script;

/**
 * @brief Open a script from a loaded file
 * 
 * @param handle The file handle containing the script data
 * @returns An instance of the loaded script, `null` if loading failed
 */
BARANIUMAPI baranium_script* baranium_open_script(baranium_handle* handle);

/**
 * @brief Safely close & dispose a script and it's data
 * 
 * @param script The script that will be disposed
 */
BARANIUMAPI void baranium_close_script(baranium_script* script);

/**
 * @brief Get a section by it's ID and check if it has the desired type
 * 
 * @param script The script containing the section
 * @param id ID of the section
 * @param type Desired type of the section
 * @returns A pointer to the section
 */
BARANIUMAPI baranium_section* baranium_script_get_section_by_id_and_type(baranium_script* script, index_t id, baranium_section_type_t type);

/**
 * @brief Get the location/index of a specific item with requested `name`
 * 
 * @param script The script that contains the requested item
 * @param name The name of the item
 * 
 * @returns An index to that specific item
 */
BARANIUMAPI index_t baranium_script_get_id_of(baranium_script* script, const char* name);

/**
 * @brief Get the name of a specific item with requested `id`
 * 
 * @param script The script that contains the requested item
 * @param id The id of the item
 * 
 * @returns The name of that specific item
 */
BARANIUMAPI char* baranium_script_get_name_of(baranium_script* script, index_t id);

/**
 * @brief Get a variable from a script
 * 
 * @note DISPOSE THE VARIABLE AFTERWARDS USING `baranium_variable_dispose`,
 *       this will not delete the variable from the script but rather
 *       clear the allocated memory
 * 
 * @param script The script containing the variable
 * @param name The ID of the variable
 * 
 * @returns The found variable, `NULL` if not found
 */
BARANIUMAPI baranium_variable* baranium_script_get_variable(baranium_script* script, const char* name);

/**
 * @brief Get a variable from a script
 * 
 * @note DISPOSE THE VARIABLE AFTERWARDS USING `baranium_variable_dispose`,
 *       this will not delete the variable from the script but rather
 *       clear the allocated memory
 * 
 * @param script The script containing the variable
 * @param variableID The ID of the variable
 * 
 * @returns The found variable, `NULL` if not found
 */
BARANIUMAPI baranium_variable* baranium_script_get_variable_by_id(baranium_script* script, index_t variableID);

/**
 * @brief Get a field from a script
 * 
 * @note DISPOSE THE FIELD AFTERWARDS USING `baranium_field_dispose`,
 *       this will not delete the field from the script but rather
 *       clear the allocated memory
 * 
 * @param script The script containing the field
 * @param name The name of the field
 * 
 * @returns The found field, `NULL` if not found
 */
BARANIUMAPI baranium_field* baranium_script_get_field(baranium_script* script, const char* name);

/**
 * @brief Get a field from a script
 * 
 * @note DISPOSE THE FIELD AFTERWARDS USING `baranium_field_dispose`,
 *       this will not delete the field from the script but rather
 *       clear the allocated memory
 * 
 * @param script The script containing the field
 * @param fieldID The ID of the field
 * 
 * @returns The found field, `NULL` if not found
 */
BARANIUMAPI baranium_field* baranium_script_get_field_by_id(baranium_script* script, index_t fieldID);

/**
 * @brief Get a function from a script
 * 
 * @note DISPOSE THE FUNCTION AFTERWARDS USING `baranium_function_dispose`,
 *       this will not delete the function from the script but rather
 *       clear the allocated memory
 * 
 * @param script The script containing the function
 * @param name The function name
 * 
 * @returns The found function, `NULL` if not found
 */
BARANIUMAPI baranium_function* baranium_script_get_function(baranium_script* script, const char* name);

/**
 * @brief Get a function from a script
 * 
 * @param script The script containing the function
 * @param functionID The function ID
 * 
 * @returns The found function, `NULL` if not found
 */
BARANIUMAPI baranium_function* baranium_script_get_function_by_id(baranium_script* script, index_t functionID);

#ifdef __cplusplus
}
#endif

#endif
