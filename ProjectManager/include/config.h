/**
 * @file config.h
 * @author GAMINGNOOBdev (https://github.com/GAMINGNOOBdev)
 * @brief A simple toml-like config file parser/writer utility
 * @version 1.0
 * @date 2025-07-19
 * 
 * @note Define CONFIG_IMPLEMENTATION in ONE!! file where you include this header
 * 
 * @copyright Copyright (c) GAMINGNOOBdev 2025
 */
#ifndef __CONFIG_H_
#define __CONFIG_H_

#include <stdint.h>
#include <stdio.h>

///////////////////
///             ///
///   Defines   ///
///             ///
///////////////////

#define CONFIG_PROPERTY_INIT(_name, _type, _value, _arrayLength, _parent) (config_property_t){.name=_name, .type=_type, .value=_value, .arrayLength=_arrayLength, .parent=_parent}
#define CONFIG_PROPERTY_EMPTY CONFIG_PROPERTY_INIT(0, 0, {0}, 0, 0)
#define CONFIG_SECTION_EMPTY (config_section_t){.name=(const char*)0, .parent=(config_section_t*)0, .sections=(config_section_t*)0, .sectionCount=0, .properties=(config_property_t*)0, .propertyCount=0}
#define CONFIG_FILE_EMPTY (config_file_t){.sections=(config_section_t*)0, .sectionCount=0, .properties=(config_property_t*)0, .propertyCount=0}

#define FOREACH_PROPERTY(name, containerptr, handle) for (size_t i##name = 0; i##name < containerptr->propertyCount; i##name++) { config_property_t* name = &containerptr->properties[i##name]; handle }
#define FOREACH_SECTION(name, containerptr, handle) for (size_t i##name = 0; i##name < containerptr->sectionCount; i##name++) { config_section_t* name = &containerptr->sections[i##name]; handle }

#define CONFIG_PROPERTY_VALUE_TYPE_UNKNOWN  ((int)0)
#define CONFIG_PROPERTY_VALUE_TYPE_STRING   ((int)1)
#define CONFIG_PROPERTY_VALUE_TYPE_FLOAT    ((int)2)
#define CONFIG_PROPERTY_VALUE_TYPE_BOOL     ((int)3)
#define CONFIG_PROPERTY_VALUE_TYPE_INT      ((int)4)
#define CONFIG_PROPERTY_VALUE_TYPE_ARRAY    ((int)5)

struct config_section_t;

//////////////////////////
///                    ///
///   Util functions   ///
///                    ///
//////////////////////////

/**
 * @brief Estimate/guess value type from string
 * 
 * @param value Value string
 * @returns Value type
 */
int config_estimate_value_type(const char* value);

/**
 * @brief Config property type as string
 * 
 * @param type Type
 * @returns Type name as string
 */
const char* config_property_value_type_as_string(int type);

///////////////////////////
///                     ///
///   Config Property   ///
///                     ///
///////////////////////////

typedef struct config_property_t
{
    const char* name;
    int type;
    union {
        struct config_property_t* arrayValue;
        const char* stringValue;
        float floatValue;
        int intValue;
        uint8_t boolValue;
    } value;
    int arrayLength;

    struct config_section_t* parent;
} config_property_t;

/**
 * @brief Dispose a config property
 * 
 * @param property Config property
 */
void config_property_dispose(config_property_t* property);

/**
 * @brief Duplicates all contents of a property
 * 
 * @note Returned property has to be disposed with `config_property_dispose()`
 * 
 * @param property Config property
 * @returns The property with all duplicated contents
 */
config_property_t config_property_duplicate(config_property_t* property);

/**
 * @brief Get full config property name
 * 
 * @note Returned string has to be manually cleaned up by the used using `free()`
 * 
 * @param property Config property
 * @returns Property name
 */
const char* config_property_get_full_name(config_property_t* property);

/**
 * @brief Add an array element to this property (if it is of the array type)
 * 
 * @param property Config property
 * @param other Other property
 */
void config_property_array_add(config_property_t* property, config_property_t* other);

/**
 * @brief Insert an array element to this property at the given index (pushes values at the index towards the end) (if it is of the array type)
 * 
 * @param property Config property
 * @param index Index
 * @param other Other property
 */
void config_property_array_insert_at(config_property_t* property, int index, config_property_t* other);

/**
 * @brief Remove an array element from this property (if it is of the array type)
 * 
 * @param property Config property
 * @param index Index
 */
void config_property_array_remove(config_property_t* property, int index);

/**
 * @brief Set config property value from a string
 * 
 * @note `value` should be modifyable and will stay modified after use
 * 
 * @param property Config property
 * @param value Value string
 */
void config_property_set_value_from_string(config_property_t* property, const char* value);

/**
 * @brief Set config property value
 * 
 * @note `value` should be modifyable and will stay modified after use
 * 
 * @param property Config property
 * @param value Value
 */
void config_property_set_string(config_property_t* property, const char* value);

/**
 * @brief Set config property value
 * 
 * @param property Config property
 * @param value Value
 */
void config_property_set_float(config_property_t* property, float value);

/**
 * @brief Set config property value
 * 
 * @param property Config property
 * @param value Value
 */
void config_property_set_bool(config_property_t* property, uint8_t value);

/**
 * @brief Set config property value
 * 
 * @param property Config property
 * @param value Value
 */
void config_property_set_int(config_property_t* property, int value);

/**
 * @brief Set config property value
 * 
 * @param property Config property
 * @param len Array length
 * @param array Array of values
 */
void config_property_set_array(config_property_t* property, int len, config_property_t array[]);

/**
 * @brief Save property to a file
 * 
 * @param property Config property
 * @param file Output file
 */
void config_property_save(config_property_t* property, FILE* file);

/**
 * @brief Compare two properties to each other
 * 
 * @param a First property
 * @param b Second property
 * @returns Comparison result, 0 if equal
 */
int config_property_cmp(config_property_t* a, config_property_t* b);

//////////////////////////
///                    ///
///   Config Section   ///
///                    ///
//////////////////////////

typedef struct config_section_t
{
    const char* name;
    struct config_section_t* parent;

    struct config_section_t* sections;
    size_t sectionCount;

    struct config_property_t* properties;
    size_t propertyCount;
} config_section_t;

/**
 * @brief Dispose a config section
 * 
 * @param section Config section
 */
void config_section_dispose(config_section_t* section);

/**
 * @brief Get full config section name
 * 
 * @note Returned string has to be manually cleaned up by the used using `free()`
 * 
 * @param section Config section
 * @returns Section name
 */
const char* config_section_get_full_name(config_section_t* section);

/**
 * @brief Save config section to a file
 * 
 * @param section Config section
 * @param file Output file
 * @param prefix Section name prefix
 */
void config_section_save(config_section_t* section, FILE* file, const char* prefix);

/**
 * @brief Get a property inside this section
 * 
 * @param section Config section
 * @param name Property name
 * @returns The desired config property
 */
config_property_t* config_section_get(config_section_t* section, const char* name);

/**
 * @brief Get a section inside this section
 * 
 * @param section Config section
 * @param name Section name
 * @returns The desired config section
 */
config_section_t* config_section_get_section(config_section_t* section, const char* name);

/**
 * @brief Get whether a property exists in this section
 * 
 * @param section Config section
 * @param name Property name
 * @returns 0 if no, 1 if yes
 */
int config_section_has_property(config_section_t* section, const char* name);

/**
 * @brief Get whether a section exists in this section
 * 
 * @param section Config section
 * @param name Section name
 * @returns 0 if no, 1 if yes
 */
int config_section_has_section(config_section_t* section, const char* name);

/**
 * @brief Add a property to this section
 * 
 * @param section Config section
 * @param name Property name
 * @returns The newly created property
 */
config_property_t* config_section_add_property(config_section_t* section, const char* name);

/**
 * @brief Add a section to this section
 * 
 * @param section Config section
 * @param name Section name
 * @returns The newly created section
 */
config_section_t* config_section_add_section(config_section_t* section, const char* name);

/**
 * @brief Remove a property from this section
 * 
 * @param section Config section
 * @param name Property name
 */
void config_section_remove_property(config_section_t* section, const char* name);

/**
 * @brief Remove a section from this section
 * 
 * @param section Config section
 * @param name Section name
 */
void config_section_remove_section(config_section_t* section, const char* name);

/**
 * @brief Compare two sections to each other
 * 
 * @param a First section
 * @param b Second section
 * @returns Comparison result, 0 if equal
 */
int config_section_cmp(config_section_t* a, config_section_t* b);

///////////////////////
///                 ///
///   Config File   ///
///                 ///
///////////////////////

typedef struct config_file_t
{
    struct config_section_t* sections;
    size_t sectionCount;

    struct config_property_t* properties;
    size_t propertyCount;
} config_file_t;

/**
 * @brief Close a config file
 * 
 * @note Should ALWAYS be called after completing usage of a `config_file_t` object
 * 
 * @param config Config file
 */
void config_file_close(config_file_t* config);

/**
 * @brief Open and parse a config file
 * 
 * @param config Config file
 * @param file Input file
 */
void config_file_open(config_file_t* config, FILE* file);

/**
 * @brief Save a config file
 * 
 * @param config Config file
 * @param file Output file
 */
void config_file_save(config_file_t* config, FILE* file);

/**
 * @brief Get a property inside this file
 * 
 * @param config Config file
 * @param name Property name
 * @returns The desired config property
 */
config_property_t* config_file_get(config_file_t* config, const char* name);

/**
 * @brief Get a section inside this file
 * 
 * @param config Config file
 * @param name Section name
 * @returns The desired config section
 */
config_section_t* config_file_get_section(config_file_t* config, const char* name);

/**
 * @brief Get whether a property exists in this file
 * 
 * @param config Config file
 * @param name Property name
 * @returns 0 if no, 1 if yes
 */
int config_file_has_property(config_file_t* config, const char* name);

/**
 * @brief Get whether a section exists in this file
 * 
 * @param config Config file
 * @param name Section name
 * @returns 0 if no, 1 if yes
 */
int config_file_has_section(config_file_t* config, const char* name);

/**
 * @brief Add a property to this file
 * 
 * @param config Config file
 * @param name Property name
 * @returns The newly created property
 */
config_property_t* config_file_add_property(config_file_t* config, const char* name);

/**
 * @brief Add a section to this file
 * 
 * @param config Config file
 * @param name Section name
 * @returns The newly created section
 */
config_section_t* config_file_add_section(config_file_t* config, const char* name);

/**
 * @brief Remove a property from this file
 * 
 * @param config Config file
 * @param name Property name
 */
void config_file_remove_property(config_file_t* config, const char* name);

/**
 * @brief Remove a section from this file
 * 
 * @param config Config file
 * @param name Section name
 */
void config_file_remove_section(config_file_t* config, const char* name);

#endif
