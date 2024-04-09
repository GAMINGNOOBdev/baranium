#ifndef __BARANIUM__SCRIPT_H_
#define __BARANIUM__SCRIPT_H_ 1

#include <stdint.h>
#include "defines.h"
#include "field.h"
#include "variable.h"
#include "function.h"

#define MAGIC_NUM_0 'B'
#define MAGIC_NUM_1 'G'
#define MAGIC_NUM_2 'S'
#define MAGIC_NUM_3 'L'

enum BaraniumSectionType
{
    BaraniumSectionType_Invalid,
    BaraniumSectionType_Fields,
    BaraniumSectionType_Variables,
    BaraniumSectionType_Functions,
};

typedef struct BaraniumSection
{
    struct BaraniumSection* prev;

    uint8_t Type;
    index_t ID;
    uint64_t DataSize;
    uint8_t* Data;

    struct BaraniumSection* next;
} BaraniumSection;

typedef struct BaraniumScriptHeader
{
    uint8_t MagicNumber[4];
    uint32_t Version;
    uint64_t SectionCount;
} BaraniumScriptHeader;

typedef struct BaraniumScriptNameTableEntry
{
    struct BaraniumScriptNameTableEntry* prev;

    uint8_t NameLength;
    uint8_t* Name;
    index_t ID;

    struct BaraniumScriptNameTableEntry* next;
} BaraniumScriptNameTableEntry;

typedef struct BaraniumScriptNameTable
{
    uint64_t NameCount;
    BaraniumScriptNameTableEntry* EntriesStart;
    BaraniumScriptNameTableEntry* EntriesEnd;
} BaraniumScriptNameTable;

typedef struct BaraniumScript
{
    BaraniumScriptHeader Header;
    BaraniumSection* SectionsStart;
    BaraniumSection* SectionsEnd;
    BaraniumScriptNameTable NameTable;
} BaraniumScript;

/**
 * @brief Open a script from a loaded file
 * 
 * @param handle The file handle containing the script data
 * @returns An instance of the loaded script, `null` if loading failed
 */
BARANIUMAPI BaraniumScript* baranium_open_script(BaraniumHandle* handle);

/**
 * @brief Safely close & dispose a script and it's data
 * 
 * @param script The script that will be disposed
 */
BARANIUMAPI void baranium_close_script(BaraniumScript* script);

/**
 * @brief Get a section by it's ID and check if it has the desired type
 * 
 * @param script The script containing the section
 * @param id ID of the section
 * @param type Desired type of the section
 * @returns A pointer to the section
 */
BARANIUMAPI BaraniumSection* baranium_script_get_section_by_id_and_type(BaraniumScript* script, index_t id, enum BaraniumSectionType type);

/**
 * @brief Get the location/index of a specific item with requested `name`
 * 
 * @param script The script that contains the requested item
 * @param name The name of the item
 * 
 * @returns An index to that specific item
 */
BARANIUMAPI index_t baranium_script_get_id_of(BaraniumScript* script, const char* name);

/**
 * @brief Get the name of a specific item with requested `id`
 * 
 * @param script The script that contains the requested item
 * @param id The id of the item
 * 
 * @returns The name of that specific item
 */
BARANIUMAPI char* baranium_script_get_name_of(BaraniumScript* script, index_t id);

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
BARANIUMAPI BaraniumVariable* baranium_script_get_variable(BaraniumScript* script, const char* name);

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
BARANIUMAPI BaraniumVariable* baranium_script_get_variable_by_id(BaraniumScript* script, index_t variableID);

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
BARANIUMAPI BaraniumField* baranium_script_get_field(BaraniumScript* script, const char* name);

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
BARANIUMAPI BaraniumField* baranium_script_get_field_by_id(BaraniumScript* script, index_t fieldID);

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
BARANIUMAPI BaraniumFunction* baranium_script_get_function(BaraniumScript* script, const char* name);

/**
 * @brief Get a function from a script
 * 
 * @param script The script containing the function
 * @param functionID The function ID
 * 
 * @returns The found function, `NULL` if not found
 */
BARANIUMAPI BaraniumFunction* baranium_script_get_function_by_id(BaraniumScript* script, index_t functionID);

#endif