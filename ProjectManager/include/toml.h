/**
 * @file toml.h
 * @author GAMINGNOOBdev (https://github.com/GAMINGNOOBdev)
 * @brief A simple toml-like config file parser/writer utility
 * @version 1.0
 * 
 * @note Define TOML_IMPLEMENTATION in ONE!! file where you include this header
 * 
 * @copyright Copyright (c) GAMINGNOOBdev
 */
#ifndef __TOML_H_
#define __TOML_H_

#include <stdint.h>
#include <stdio.h>

///////////////////
///             ///
///   Defines   ///
///             ///
///////////////////

#define TOML_PROPERTY_INIT(_name, _type, _value, _arrayLength, _parent) (toml_property_t){.name=_name, .type=_type, .value=_value, .arrayLength=_arrayLength, .parent=_parent}
#define TOML_PROPERTY_EMPTY TOML_PROPERTY_INIT(0, 0, {0}, 0, 0)
#define TOML_SECTION_EMPTY (toml_section_t){.name=(const char*)0, .parent=(toml_section_t*)0, .sections=(toml_section_t*)0, .sectionCount=0, .properties=(toml_property_t*)0, .propertyCount=0}
#define TOML_FILE_EMPTY (toml_file_t){.sections=(toml_section_t*)0, .sectionCount=0, .properties=(toml_property_t*)0, .propertyCount=0}

#define FOREACH_PROPERTY(name, containerptr, handle) for (size_t i##name = 0; i##name < containerptr->propertyCount; i##name++) { toml_property_t* name = &containerptr->properties[i##name]; handle }
#define FOREACH_SECTION(name, containerptr, handle) for (size_t i##name = 0; i##name < containerptr->sectionCount; i##name++) { toml_section_t* name = &containerptr->sections[i##name]; handle }

#define TOML_PROPERTY_VALUE_TYPE_UNKNOWN  ((int)0)
#define TOML_PROPERTY_VALUE_TYPE_STRING   ((int)1)
#define TOML_PROPERTY_VALUE_TYPE_FLOAT    ((int)2)
#define TOML_PROPERTY_VALUE_TYPE_BOOL     ((int)3)
#define TOML_PROPERTY_VALUE_TYPE_INT      ((int)4)
#define TOML_PROPERTY_VALUE_TYPE_ARRAY    ((int)5)

struct toml_section_t;

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
int toml_estimate_value_type(const char* value);

/**
 * @brief Config property type as string
 * 
 * @param type Type
 * @returns Type name as string
 */
const char* toml_property_value_type_as_string(int type);

/////////////////////////
///                   ///
///   Toml Property   ///
///                   ///
/////////////////////////

typedef struct toml_property_t
{
    const char* name;
    int type;
    union {
        struct toml_property_t* arrayValue;
        const char* stringValue;
        float floatValue;
        int intValue;
        uint8_t boolValue;
    } value;
    int arrayLength;

    struct toml_section_t* parent;
} toml_property_t;

/**
 * @brief Dispose a config property
 * 
 * @param property Config property
 */
void toml_property_dispose(toml_property_t* property);

/**
 * @brief Duplicates all contents of a property
 * 
 * @note Returned property has to be disposed with `toml_property_dispose()`
 * 
 * @param property Config property
 * @returns The property with all duplicated contents
 */
toml_property_t toml_property_duplicate(toml_property_t* property);

/**
 * @brief Get full config property name
 * 
 * @note Returned string has to be manually cleaned up by the used using `free()`
 * 
 * @param property Config property
 * @returns Property name
 */
const char* toml_property_get_full_name(toml_property_t* property);

/**
 * @brief Add an array element to this property (if it is of the array type)
 * 
 * @param property Config property
 * @param other Other property
 */
void toml_property_array_add(toml_property_t* property, toml_property_t* other);

/**
 * @brief Insert an array element to this property at the given index (pushes values at the index towards the end) (if it is of the array type)
 * 
 * @param property Config property
 * @param index Index
 * @param other Other property
 */
void toml_property_array_insert_at(toml_property_t* property, int index, toml_property_t* other);

/**
 * @brief Remove an array element from this property (if it is of the array type)
 * 
 * @param property Config property
 * @param index Index
 */
void toml_property_array_remove(toml_property_t* property, int index);

/**
 * @brief Set config property value from a string
 * 
 * @note `value` should be modifyable and will stay modified after use
 * 
 * @param property Config property
 * @param value Value string
 */
void toml_property_set_value_from_string(toml_property_t* property, const char* value);

/**
 * @brief Set config property value
 * 
 * @note `value` should be modifyable and will stay modified after use
 * 
 * @param property Config property
 * @param value Value
 */
void toml_property_set_string(toml_property_t* property, const char* value);

/**
 * @brief Set config property value
 * 
 * @param property Config property
 * @param value Value
 */
void toml_property_set_float(toml_property_t* property, float value);

/**
 * @brief Set config property value
 * 
 * @param property Config property
 * @param value Value
 */
void toml_property_set_bool(toml_property_t* property, uint8_t value);

/**
 * @brief Set config property value
 * 
 * @param property Config property
 * @param value Value
 */
void toml_property_set_int(toml_property_t* property, int value);

/**
 * @brief Set config property value
 * 
 * @param property Config property
 * @param len Array length
 * @param array Array of values
 */
void toml_property_set_array(toml_property_t* property, int len, toml_property_t array[]);

/**
 * @brief Save property to a file
 * 
 * @param property Config property
 * @param file Output file
 */
void toml_property_save(toml_property_t* property, FILE* file);

/**
 * @brief Compare two properties to each other
 * 
 * @param a First property
 * @param b Second property
 * @returns Comparison result, 0 if equal
 */
int toml_property_cmp(toml_property_t* a, toml_property_t* b);

////////////////////////
///                  ///
///   Toml Section   ///
///                  ///
////////////////////////

typedef struct toml_section_t
{
    const char* name;
    struct toml_section_t* parent;

    struct toml_section_t* sections;
    size_t sectionCount;

    struct toml_property_t* properties;
    size_t propertyCount;
} toml_section_t;

/**
 * @brief Dispose a config section
 * 
 * @param section Config section
 */
void toml_section_dispose(toml_section_t* section);

/**
 * @brief Get full config section name
 * 
 * @note Returned string has to be manually cleaned up by the used using `free()`
 * 
 * @param section Config section
 * @returns Section name
 */
const char* toml_section_get_full_name(toml_section_t* section);

/**
 * @brief Save config section to a file
 * 
 * @param section Config section
 * @param file Output file
 * @param prefix Section name prefix
 */
void toml_section_save(toml_section_t* section, FILE* file, const char* prefix);

/**
 * @brief Get a property inside this section
 * 
 * @param section Config section
 * @param name Property name
 * @returns The desired config property
 */
toml_property_t* toml_section_get(toml_section_t* section, const char* name);

/**
 * @brief Get a section inside this section
 * 
 * @param section Config section
 * @param name Section name
 * @returns The desired config section
 */
toml_section_t* toml_section_get_section(toml_section_t* section, const char* name);

/**
 * @brief Get whether a property exists in this section
 * 
 * @param section Config section
 * @param name Property name
 * @returns 0 if no, 1 if yes
 */
int toml_section_has_property(toml_section_t* section, const char* name);

/**
 * @brief Get whether a section exists in this section
 * 
 * @param section Config section
 * @param name Section name
 * @returns 0 if no, 1 if yes
 */
int toml_section_has_section(toml_section_t* section, const char* name);

/**
 * @brief Add a property to this section
 * 
 * @param section Config section
 * @param name Property name
 * @returns The newly created property
 */
toml_property_t* toml_section_add_property(toml_section_t* section, const char* name);

/**
 * @brief Add a section to this section
 * 
 * @param section Config section
 * @param name Section name
 * @returns The newly created section
 */
toml_section_t* toml_section_add_section(toml_section_t* section, const char* name);

/**
 * @brief Remove a property from this section
 * 
 * @param section Config section
 * @param name Property name
 */
void toml_section_remove_property(toml_section_t* section, const char* name);

/**
 * @brief Remove a section from this section
 * 
 * @param section Config section
 * @param name Section name
 */
void toml_section_remove_section(toml_section_t* section, const char* name);

/**
 * @brief Compare two sections to each other
 * 
 * @param a First section
 * @param b Second section
 * @returns Comparison result, 0 if equal
 */
int toml_section_cmp(toml_section_t* a, toml_section_t* b);

///////////////////////
///                 ///
///   Config File   ///
///                 ///
///////////////////////

typedef struct toml_file_t
{
    struct toml_section_t* sections;
    size_t sectionCount;

    struct toml_property_t* properties;
    size_t propertyCount;
} toml_file_t;

/**
 * @brief Close a config file
 * 
 * @note Should ALWAYS be called after completing usage of a `toml_file_t` object
 * 
 * @param config Config file
 */
void toml_file_close(toml_file_t* config);

/**
 * @brief Open and parse a config file
 * 
 * @param config Config file
 * @param file Input file
 */
void toml_file_open(toml_file_t* config, FILE* file);

/**
 * @brief Save a config file
 * 
 * @param config Config file
 * @param file Output file
 */
void toml_file_save(toml_file_t* config, FILE* file);

/**
 * @brief Get a property inside this file
 * 
 * @param config Config file
 * @param name Property name
 * @returns The desired config property
 */
toml_property_t* toml_file_get(toml_file_t* config, const char* name);

/**
 * @brief Get a section inside this file
 * 
 * @param config Config file
 * @param name Section name
 * @returns The desired config section
 */
toml_section_t* toml_file_get_section(toml_file_t* config, const char* name);

/**
 * @brief Get whether a property exists in this file
 * 
 * @param config Config file
 * @param name Property name
 * @returns 0 if no, 1 if yes
 */
int toml_file_has_property(toml_file_t* config, const char* name);

/**
 * @brief Get whether a section exists in this file
 * 
 * @param config Config file
 * @param name Section name
 * @returns 0 if no, 1 if yes
 */
int toml_file_has_section(toml_file_t* config, const char* name);

/**
 * @brief Add a property to this file
 * 
 * @param config Config file
 * @param name Property name
 * @returns The newly created property
 */
toml_property_t* toml_file_add_property(toml_file_t* config, const char* name);

/**
 * @brief Add a section to this file
 * 
 * @param config Config file
 * @param name Section name
 * @returns The newly created section
 */
toml_section_t* toml_file_add_section(toml_file_t* config, const char* name);

/**
 * @brief Remove a property from this file
 * 
 * @param config Config file
 * @param name Property name
 */
void toml_file_remove_property(toml_file_t* config, const char* name);

/**
 * @brief Remove a section from this file
 * 
 * @param config Config file
 * @param name Section name
 */
void toml_file_remove_section(toml_file_t* config, const char* name);

#ifdef TOML_IMPLEMENTATION

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//////////////////////////
///                    ///
///   Util functions   ///
///                    ///
//////////////////////////

// free a block of memory IF not null
void iffree(void* block)
{
    if (!block)
        return;

    free(block);
}

// modifies the given `src` string (no realloc)
void strtrim(char* src)
{
    if (src == NULL)
        return;

    int len = strlen(src);
    uint8_t index = 0;
    for (int i = 0; i < len; i++)
    {
        index = !isspace(src[i]) ? 1 : index;
        if (index)
            break;
    }

    if (!index)
    {
        src[0] = 0;
        return;
    }

    int begin_index = 0;
    int end_index = 0;

    for (begin_index = 0; begin_index < len; begin_index++)
    {
        if (!isspace(src[begin_index]))
            break;
    }
    for (end_index = len-1; end_index > 0; end_index--)
    {
        if (!isspace(src[end_index]))
            break;
    }

    if (begin_index == 0 && end_index == len-1)
        return;

    if (end_index == 0 && begin_index == len)
    {
        src[0] = 0;
        return;
    }

    len = end_index - begin_index + 1;
    for (int i = 0; i < len; i++)
        src[i] = src[i + begin_index];
    src[len] = 0;
}

int is_bool(const char* str)
{
    if (strcmp(str, "True") == 0 || strcmp(str, "true") == 0 ||
        strcmp(str, "False") == 0 || strcmp(str, "false") == 0)
        return 1;

    return 0;
}

int is_integer(const char* str)
{
    size_t index = 0;
    char chr = str[index];
    size_t length = strlen(str);
    if (chr == '+' || chr == '-')
        index++;

    for (; index < length; index++)
    {
        chr = str[index];

        if (chr < '0' || chr > '9')
            return 0;
    }

    return 1;
}

int is_float(const char* str)
{
    size_t index = 0;
    char chr = str[index];
    size_t length = strlen(str);
    if (chr == '+' || chr == '-')
        index++;

    uint8_t hasDot = 0;

    for (; index < length; index++)
    {
        chr = str[index];

        if (chr == '.')
        {
            if (hasDot)
                return 0;

            hasDot = 1;
            continue;
        }

        if (chr < '0' || chr > '9')
            return 0;
    }

    return 1;
}

int toml_estimate_value_type(const char* value)
{
    size_t valueLen = strlen(value);
    if (value == NULL || valueLen == 0)
        return TOML_PROPERTY_VALUE_TYPE_UNKNOWN;

    if (is_bool(value))
        return TOML_PROPERTY_VALUE_TYPE_BOOL;

    if (is_integer(value))
        return TOML_PROPERTY_VALUE_TYPE_INT;

    if (is_float(value))
        return TOML_PROPERTY_VALUE_TYPE_FLOAT;

    if (value[0] == '[' && value[valueLen-1] == ']')
        return TOML_PROPERTY_VALUE_TYPE_ARRAY;

    if ((value[0] == '\'' || value[0] == '"') && (value[valueLen-1] == '\'' || value[valueLen-1] == '"'))
        return TOML_PROPERTY_VALUE_TYPE_STRING;

    return TOML_PROPERTY_VALUE_TYPE_STRING;
}

const char* toml_property_value_type_as_string(int type)
{
    if (type == TOML_PROPERTY_VALUE_TYPE_STRING)
        return "STRING";

    if (type == TOML_PROPERTY_VALUE_TYPE_FLOAT)
        return "FLOAT";

    if (type == TOML_PROPERTY_VALUE_TYPE_BOOL)
        return "BOOL";

    if (type == TOML_PROPERTY_VALUE_TYPE_INT)
        return "INT";

    if (type == TOML_PROPERTY_VALUE_TYPE_ARRAY)
        return "ARRAY";

    return "UNKNOWN";
}

///////////////////////////
///                     ///
///   Config Property   ///
///                     ///
///////////////////////////

void toml_property_dispose(toml_property_t* property)
{
    if (property == NULL)
        return;

    iffree((void*)property->name);

    for (int i = 0; i < property->arrayLength; i++)
        toml_property_dispose(&property->value.arrayValue[i]);

    if (property->type == TOML_PROPERTY_VALUE_TYPE_STRING || property->type == TOML_PROPERTY_VALUE_TYPE_UNKNOWN)
        iffree((void*)property->value.stringValue);

    if (property->type == TOML_PROPERTY_VALUE_TYPE_ARRAY)
        iffree((void*)property->value.arrayValue);

    memset(property, 0, sizeof(toml_property_t));
}

toml_property_t toml_property_duplicate(toml_property_t* property)
{
    toml_property_t result = TOML_PROPERTY_EMPTY;
    if (property == NULL)
        return result;

    if (property->name)
        result.name = strdup(property->name);
    result.arrayLength = property->arrayLength;
    result.type = property->type;
    result.parent = property->parent;
    result.value = property->value;

    if (result.type == TOML_PROPERTY_VALUE_TYPE_ARRAY)
    {
        result.value.arrayValue = (toml_property_t*)malloc(sizeof(toml_property_t)*result.arrayLength);
        for (int i = 0; i < result.arrayLength; i++)
            result.value.arrayValue[i] = toml_property_duplicate(&property->value.arrayValue[i]);
        return result;
    }

    if (result.type == TOML_PROPERTY_VALUE_TYPE_UNKNOWN || result.type == TOML_PROPERTY_VALUE_TYPE_STRING)
    {
        result.value.stringValue = strdup(result.value.stringValue);
        return result;
    }

    return result;
}

const char* toml_property_get_full_name(toml_property_t* property)
{
    if (property == NULL)
        return NULL;

    if (property->parent == NULL)
        return strdup(property->name);

    char* parentSectionName = (char*)toml_section_get_full_name(property->parent);
    char* parent = strdup(parentSectionName);
    parent = (char*)realloc(parent, strlen(parent) + strlen(property->name) + 2);
    free(parentSectionName);
    strcat(parent, ".");
    strcat(parent, property->name);
    return parent;
}

void toml_property_array_add(toml_property_t* property, toml_property_t* other)
{
    if (property == NULL || other == NULL)
        return;

    if (property->type != TOML_PROPERTY_VALUE_TYPE_ARRAY)
        return;

    property->arrayLength++;
    property->value.arrayValue = (toml_property_t*)realloc(property->value.arrayValue, sizeof(toml_property_t)*property->arrayLength);
    property->value.arrayValue[property->arrayLength-1] = toml_property_duplicate(other);
}

void toml_property_array_insert_at(toml_property_t* property, int index, toml_property_t* other)
{
    if (property == NULL || index < 0 || other == NULL)
        return;

    if (property->type != TOML_PROPERTY_VALUE_TYPE_ARRAY)
        return;

    if ((property->arrayLength == 0 && index == 0) || index == -1)
    {
        toml_property_array_add(property, other);
        return;
    }

    if (index >= property->arrayLength)
        return;

    property->arrayLength++;
    property->value.arrayValue = (toml_property_t*)realloc(property->value.arrayValue, sizeof(toml_property_t)*property->arrayLength);
    memmove(&property->value.arrayValue[index+1], &property->value.arrayValue[index], sizeof(toml_property_t)*(property->arrayLength-index-1));
    property->value.arrayValue[index] = toml_property_duplicate(other);
}

void toml_property_array_remove(toml_property_t* property, int index)
{
    if (property == NULL)
        return;

    if (property->type != TOML_PROPERTY_VALUE_TYPE_ARRAY)
        return;

    if (property->arrayLength == 0)
        return;

    if (index == -1 || index == property->arrayLength-1)
    {
        toml_property_dispose(&property->value.arrayValue[property->arrayLength-1]);
        property->arrayLength--;
        property->value.arrayValue = (toml_property_t*)realloc(property->value.arrayValue, sizeof(toml_property_t)*property->arrayLength);
        return;
    }
    else if (index == 0)
    {
        toml_property_t tmp = property->value.arrayValue[0];
        toml_property_dispose(&tmp);
        memmove(property->value.arrayValue, &property->value.arrayValue[1], sizeof(toml_property_t)*property->arrayLength-1);
        property->arrayLength--;
        property->value.arrayValue = (toml_property_t*)realloc(property->value.arrayValue, sizeof(toml_property_t)*property->arrayLength);
        return;
    }
    else if (index >= property->arrayLength)
        return;

    toml_property_t tmp = property->value.arrayValue[index];
    toml_property_dispose(&tmp);
    memmove(&property->value.arrayValue[index], &property->value.arrayValue[index+1], sizeof(toml_property_t)*(property->arrayLength-index-1));
    property->arrayLength--;
    property->value.arrayValue = (toml_property_t*)realloc(property->value.arrayValue, sizeof(toml_property_t)*property->arrayLength);
}

void toml_property_set_value_from_string(toml_property_t* property, const char* string)
{
    if (property == NULL || string == NULL)
        return;

    char* value = strdup(string);
    char* valueog = value; // in case we modify it if the `value` pointer
    strtrim(value);
    property->type = toml_estimate_value_type(value);
    char* tmp;
    switch (property->type)
    {
    case TOML_PROPERTY_VALUE_TYPE_INT:
    {
        property->value.intValue = strtol(value, &tmp, 10);
        break;
    }

    case TOML_PROPERTY_VALUE_TYPE_FLOAT:
    {
        property->value.floatValue = strtof(value, &tmp);
        break;
    }

    case TOML_PROPERTY_VALUE_TYPE_BOOL:
    {
        property->value.boolValue = 0;
        if (strcmp(value, "true") == 0 || strcmp(value, "True") == 0)
            property->value.boolValue = 1;
        break;
    }

    case TOML_PROPERTY_VALUE_TYPE_ARRAY:
    {
        if (value[0] == '[')
            value++;
        size_t valueLen = strlen(value);
        if (value[valueLen-1] == ']')
            value[valueLen-1] = 0;
        strtrim(value);

        char* value2 = strchr(value, ',');
        if (!value2)
        {
            if (strlen(value) == 0)
                break;
            property->arrayLength++;
            property->value.arrayValue = (toml_property_t*)realloc(property->value.arrayValue, sizeof(toml_property_t)*property->arrayLength);
            toml_property_t tmp = TOML_PROPERTY_EMPTY;
            toml_property_set_value_from_string(&tmp, value);
            memcpy(&property->value.arrayValue[property->arrayLength-1], &tmp, sizeof(toml_property_t));
        }
        else
        {
            value2[0] = 0;
            value2++;
            property->arrayLength++;
            property->value.arrayValue = (toml_property_t*)realloc(property->value.arrayValue, sizeof(toml_property_t)*property->arrayLength);
            toml_property_t tmp = TOML_PROPERTY_EMPTY;
            toml_property_set_value_from_string(&tmp, value);
            memcpy(&property->value.arrayValue[property->arrayLength-1], &tmp, sizeof(toml_property_t));
            for (;;)
            {
                char* value3 = strchr(value2, ',');
                if (value3 == NULL)
                    break;
                value3[0] = 0;
                value3++;
                property->arrayLength++;
                property->value.arrayValue = (toml_property_t*)realloc(property->value.arrayValue, sizeof(toml_property_t)*property->arrayLength);
                tmp = TOML_PROPERTY_EMPTY;
                toml_property_set_value_from_string(&tmp, value2);
                memcpy(&property->value.arrayValue[property->arrayLength-1], &tmp, sizeof(toml_property_t));
                value2 = value3;
            }

            property->arrayLength++;
            property->value.arrayValue = (toml_property_t*)realloc(property->value.arrayValue, sizeof(toml_property_t)*property->arrayLength);
            tmp = TOML_PROPERTY_EMPTY;
            toml_property_set_value_from_string(&tmp, value2);
            memcpy(&property->value.arrayValue[property->arrayLength-1], &tmp, sizeof(toml_property_t));
        }
        break;
    }

    case TOML_PROPERTY_VALUE_TYPE_UNKNOWN:
    case TOML_PROPERTY_VALUE_TYPE_STRING:
    {
        if (value[0] == '\'' || value[0] == '"')
            value++;
        size_t valueLen = strlen(value);
        if (value[valueLen-1] == '\'' || value[valueLen-1] == '"')
            value[valueLen-1] = 0;

        property->value.stringValue = strdup(value);
        break;
    }
    }
    free((void*)valueog);
}

void toml_property_set_string(toml_property_t* property, const char* value)
{
    if (property == NULL)
        return;

    if (property->type == TOML_PROPERTY_VALUE_TYPE_STRING || property->type == TOML_PROPERTY_VALUE_TYPE_UNKNOWN)
        iffree((void*)property->value.stringValue);

    if (property->type == TOML_PROPERTY_VALUE_TYPE_ARRAY)
        iffree((void*)property->value.arrayValue);

    property->arrayLength = 0;
    property->type = TOML_PROPERTY_VALUE_TYPE_STRING;
    if (value == NULL)
    {
        property->value.stringValue = NULL;
        return;
    }

    property->value.stringValue = strdup(value);
}

void toml_property_set_float(toml_property_t* property, float value)
{
    if (property == NULL)
        return;

    if (property->type == TOML_PROPERTY_VALUE_TYPE_STRING || property->type == TOML_PROPERTY_VALUE_TYPE_UNKNOWN)
        iffree((void*)property->value.stringValue);

    if (property->type == TOML_PROPERTY_VALUE_TYPE_ARRAY)
        iffree((void*)property->value.arrayValue);

    property->arrayLength = 0;
    property->type = TOML_PROPERTY_VALUE_TYPE_FLOAT;
    property->value.floatValue = value;
}

void toml_property_set_bool(toml_property_t* property, uint8_t value)
{
    if (property == NULL)
        return;

    if (property->type == TOML_PROPERTY_VALUE_TYPE_STRING || property->type == TOML_PROPERTY_VALUE_TYPE_UNKNOWN)
        iffree((void*)property->value.stringValue);

    if (property->type == TOML_PROPERTY_VALUE_TYPE_ARRAY)
        iffree((void*)property->value.arrayValue);

    property->arrayLength = 0;
    property->type = TOML_PROPERTY_VALUE_TYPE_BOOL;
    property->value.boolValue = value;
}

void toml_property_set_int(toml_property_t* property, int value)
{
    if (property == NULL)
        return;

    if (property->type == TOML_PROPERTY_VALUE_TYPE_STRING || property->type == TOML_PROPERTY_VALUE_TYPE_UNKNOWN)
        iffree((void*)property->value.stringValue);

    if (property->type == TOML_PROPERTY_VALUE_TYPE_ARRAY)
        iffree((void*)property->value.arrayValue);

    property->arrayLength = 0;
    property->type = TOML_PROPERTY_VALUE_TYPE_INT;
    property->value.intValue = value;
}

void toml_property_set_array(toml_property_t* property, int len, toml_property_t array[])
{
    if (property == NULL)
        return;

    if (property->type == TOML_PROPERTY_VALUE_TYPE_STRING || property->type == TOML_PROPERTY_VALUE_TYPE_UNKNOWN)
        iffree((void*)property->value.stringValue);

    if (property->type == TOML_PROPERTY_VALUE_TYPE_ARRAY)
        iffree((void*)property->value.arrayValue);

    property->arrayLength = len;
    property->type = TOML_PROPERTY_VALUE_TYPE_ARRAY;
    property->value.arrayValue = (toml_property_t*)malloc(sizeof(toml_property_t)*len);
    for (int i = 0; i < len; i++)
        property->value.arrayValue[i] = toml_property_duplicate(&array[i]);
}

void toml_property_save_value(toml_property_t* property, FILE* file)
{
    if (property == NULL || file == NULL)
        return;

    switch (property->type)
    {
    case TOML_PROPERTY_VALUE_TYPE_INT:
    {
        fprintf(file, "%d", property->value.intValue);
        break;
    }

    case TOML_PROPERTY_VALUE_TYPE_FLOAT:
    {
        fprintf(file, "%f", property->value.floatValue);
        break;
    }

    case TOML_PROPERTY_VALUE_TYPE_BOOL:
    {
        fprintf(file, "%s", property->value.boolValue ? "true" : "false");
        break;
    }

    case TOML_PROPERTY_VALUE_TYPE_ARRAY:
    {
        fprintf(file, "[ ");
        for (int i = 0; i < property->arrayLength; i++)
        {
            toml_property_t* tmp = &property->value.arrayValue[i];
            toml_property_save_value(tmp, file);
            if (i != property->arrayLength - 1)
                fprintf(file, ", ");
        }
        fprintf(file, " ]");
        break;
    }

    case TOML_PROPERTY_VALUE_TYPE_UNKNOWN:
    case TOML_PROPERTY_VALUE_TYPE_STRING:
    {
        if (property->value.stringValue == NULL || strlen(property->value.stringValue) < 1)
            fprintf(file, "''");
        else
            fprintf(file, "'%s'", property->value.stringValue);
        break;
    }
    }
}

void toml_property_save(toml_property_t* property, FILE* file)
{
    if (property == NULL || file == NULL)
        return;

    fprintf(file, "%s = ", property->name);
    toml_property_save_value(property, file);
    fprintf(file, "\n");
    fflush(file);
}

int toml_property_cmp(toml_property_t* a, toml_property_t* b)
{
    if (a == NULL || b == NULL)
        return -1;

    if (a->type != b->type)
        return 1;

    return strcmp(a->name, b->name);
}

//////////////////////////
///                    ///
///   Config Section   ///
///                    ///
//////////////////////////

void toml_section_dispose(toml_section_t* section)
{
    if (section == NULL)
        return;

    for (size_t i = 0; i < section->sectionCount; i++)
        toml_section_dispose(&section->sections[i]);

    for (size_t i = 0; i < section->propertyCount; i++)
        toml_property_dispose(&section->properties[i]);

    iffree((void*)section->name);
    iffree((void*)section->properties);
    iffree((void*)section->sections);

    memset(section, 0, sizeof(toml_section_t));
}

const char* toml_section_get_full_name(toml_section_t* section)
{
    if (section == NULL)
        return NULL;

    if (section->parent == NULL)
        return strdup(section->name);

    char* parentSectionName = (char*)toml_section_get_full_name(section->parent);
    char* parent = strdup(parentSectionName);
    parent = (char*)realloc(parent, strlen(parent) + strlen(section->name) + 2);
    free(parentSectionName);
    parent = strcat(parent, ".");
    parent = strcat(parent, section->name);
    return parent;
}

void toml_section_save(toml_section_t* section, FILE* file, const char* prefix)
{
    if (section == NULL || file == NULL)
        return;

    fprintf(file, "[");
    if (prefix != NULL)
        fprintf(file, "%s.", prefix);
    fprintf(file, "%s]\n", section->name);

    if (section->propertyCount != 0)
    {
        for (size_t i = 0; i < section->propertyCount; i++)
            toml_property_save(&section->properties[i], file);
    }
    fprintf(file, "\n");

    if (section->sectionCount == 0)
    {
        fflush(file);
        return;
    }

    char* newSectionPrefix = strdup("");
    if (prefix != NULL)
    {
        newSectionPrefix = strdup(prefix);
        newSectionPrefix = (char*)realloc(newSectionPrefix, strlen(prefix)+2);
        newSectionPrefix = strcat(newSectionPrefix, ".");
    }
    newSectionPrefix = (char*)realloc(newSectionPrefix, strlen(newSectionPrefix) + strlen(section->name) + 1);
    newSectionPrefix = strcat(newSectionPrefix, section->name);
    for (size_t i = 0; i < section->sectionCount; i++)
        toml_section_save(&section->sections[i], file, newSectionPrefix);
    free(newSectionPrefix);

    fflush(file);
}

toml_property_t* toml_section_get(toml_section_t* section, const char* name)
{
    if (section == NULL || name == NULL || strlen(name) == 0)
        return NULL;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        for (size_t i = 0; i < section->propertyCount; i++)
        {
            if (strcmp(section->properties[i].name, str) != 0)
                continue;

            free(str);
            return &section->properties[i];
        }
        free(str);
        return NULL;
    }

    sectionDivider[0] = 0;
    sectionDivider++;
    if (!toml_section_has_section(section, str))
    {
        free(str);
        return 0;
    }

    toml_section_t* subSection = toml_section_get_section(section, str);
    toml_property_t* result = toml_section_get(subSection, sectionDivider);
    free(str);
    return result;
}

toml_section_t* toml_section_get_section(toml_section_t* section, const char* name)
{
    if (section == NULL || name == NULL || strlen(name) == 0)
        return NULL;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        for (size_t i = 0; i < section->sectionCount; i++)
        {
            if (strcmp(section->sections[i].name, str) != 0)
                continue;

            free(str);
            return &section->sections[i];
        }
        free(str);
        return NULL;
    }

    sectionDivider[0] = 0;
    sectionDivider++;
    if (!toml_section_has_section(section, str))
    {
        free(str);
        return NULL;
    }

    toml_section_t* subSection = toml_section_get_section(section, str);
    toml_section_t* result = toml_section_get_section(subSection, sectionDivider);
    free(str);
    return result;
}

int toml_section_has_property(toml_section_t* section, const char* name)
{
    if (section == NULL || name == NULL || strlen(name) == 0)
        return 0;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        for (size_t i = 0; i < section->propertyCount; i++)
        {
            if (strcmp(section->properties[i].name, name) != 0)
                continue;

            free(str);
            return 1;
        }
        free(str);
        return 0;
    }

    sectionDivider[0] = 0;
    sectionDivider++;
    if (!toml_section_has_section(section, name))
    {
        free(str);
        return 0;
    }

    toml_section_t* subSection = toml_section_get_section(section, name);
    int result = toml_section_has_property(subSection, sectionDivider);
    free(str);
    return result;
}

int toml_section_has_section(toml_section_t* section, const char* name)
{
    if (section == NULL || name == NULL || strlen(name) == 0)
        return 0;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        for (size_t i = 0; i < section->sectionCount; i++)
        {
            if (strcmp(section->sections[i].name, name) != 0)
                continue;

            free(str);
            return 1;
        }
        free(str);
        return 0;
    }

    sectionDivider[0] = 0;
    sectionDivider++;
    if (!toml_section_has_section(section, str))
    {
        free(str);
        return 0;
    }

    toml_section_t* subSection = toml_section_get_section(section, str);
    int result = toml_section_has_section(subSection, sectionDivider);
    free(str);
    return result;
}

toml_property_t* toml_section_add_property(toml_section_t* section, const char* name)
{
    if (section == NULL || name == NULL || strlen(name) == 0)
        return NULL;

    if (toml_section_has_section(section, name))
        return NULL;

    if (toml_section_has_property(section, name))
        return toml_section_get(section, name);

    toml_property_t propertry = TOML_PROPERTY_EMPTY;
    toml_property_t* propertryPtr = NULL;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        propertry.name = strdup(name);
        propertry.parent = section;

        section->propertyCount++;
        section->properties = (toml_property_t*)realloc(section->properties, section->propertyCount*sizeof(toml_property_t));
        propertryPtr = &section->properties[section->propertyCount-1];
        memcpy(propertryPtr, &propertry, sizeof(toml_property_t));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        toml_section_t* nextSection = NULL;
        if (!toml_section_has_section(section, str))
            nextSection = toml_section_add_section(section, str);
        else
            nextSection = toml_section_get_section(section, str);

        if (nextSection == NULL)
        {
            free(str);
            return NULL;
        }

        propertryPtr = toml_section_add_property(nextSection, sectionDivider);
    }

    free(str);
    return propertryPtr;
}

toml_section_t* toml_section_add_section(toml_section_t* section, const char* name)
{
    if (section == NULL || name == NULL || strlen(name) == 0)
        return NULL;

    if (toml_section_has_section(section, name))
        return toml_section_get_section(section, name);

    toml_section_t newSection = TOML_SECTION_EMPTY;
    toml_section_t* newSectionPtr = NULL;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        newSection.name = strdup(name);
        newSection.parent = section;

        section->sectionCount++;
        section->sections = (toml_section_t*)realloc(section->sections, section->sectionCount*sizeof(toml_section_t));
        newSectionPtr = &section->sections[section->sectionCount-1];
        memcpy(newSectionPtr, &newSection, sizeof(toml_section_t));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        toml_section_t* nextSection = NULL;
        if (!toml_section_has_section(section, str))
            nextSection = toml_section_add_section(section, str);
        else
            nextSection = toml_section_get_section(section, str);

        if (nextSection == NULL)
        {
            free(str);
            return NULL;
        }

        newSectionPtr = toml_section_add_section(nextSection, sectionDivider);
    }

    free(str);
    return newSectionPtr;
}

void toml_section_remove_property(toml_section_t* section, const char* name)
{
    if (section == NULL || name == NULL || strlen(name) == 0)
        return;

    if (toml_section_has_section(section, name))
        return;

    if (!toml_section_has_property(section, name))
        return;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        size_t index = 0;
        for (; index < section->propertyCount; index++)
        {
            if (strcmp(name, section->properties[index].name) == 0)
                break;
        }
        toml_property_dispose(&section->properties[index]);
        if (index != section->propertyCount-1)
        {
            toml_property_t tmp = section->properties[section->propertyCount-1];
            section->properties[section->propertyCount-1] = section->properties[index];
            section->properties[index] = tmp;
        }

        section->propertyCount--;
        section->properties = (toml_property_t*)realloc(section->properties, section->propertyCount*sizeof(toml_property_t));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        toml_section_t* nextSection = toml_section_get_section(section, str);
        if (nextSection == NULL)
        {
            free(str);
            return;
        }

        toml_section_remove_property(nextSection, sectionDivider);
    }

    free(str);
}

void toml_section_remove_section(toml_section_t* section, const char* name)
{
    if (section == NULL || name == NULL || strlen(name) == 0)
        return;

    if (!toml_section_has_section(section, name))
        return;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        size_t index = 0;
        for (; index < section->sectionCount; index++)
        {
            if (strcmp(name, section->sections[index].name) == 0)
                break;
        }
        toml_section_dispose(&section->sections[index]);
        if (index != section->sectionCount-1)
        {
            toml_section_t tmp = section->sections[section->sectionCount-1];
            section->sections[section->sectionCount-1] = section->sections[index];
            section->sections[index] = tmp;
        }

        section->sectionCount--;
        section->sections = (toml_section_t*)realloc(section->sections, section->sectionCount*sizeof(toml_section_t));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        toml_section_t* nextSection = toml_section_get_section(section, str);
        if (nextSection == NULL)
        {
            free(str);
            return;
        }

        toml_section_remove_section(nextSection, sectionDivider);
    }

    free(str);
}

int toml_section_cmp(toml_section_t* a, toml_section_t* b)
{
    if (a == NULL || b == NULL)
        return -1;

    if (a->propertyCount != b->propertyCount)
        return a->propertyCount - b->propertyCount;

    if (a->sectionCount != b->sectionCount)
        return a->sectionCount - b->sectionCount;

    return strcmp(a->name, b->name);
}


///////////////////////
///                 ///
///   Config File   ///
///                 ///
///////////////////////

void toml_file_close(toml_file_t* config)
{
    if (config == NULL)
        return;

    for (size_t i = 0; i < config->sectionCount; i++)
        toml_section_dispose(&config->sections[i]);

    for (size_t i = 0; i < config->propertyCount; i++)
        toml_property_dispose(&config->properties[i]);

    iffree(config->sections);
    iffree(config->properties);

    memset(config, 0, sizeof(toml_file_t));
}

void toml_file_open(toml_file_t* config, FILE* file)
{
    if (config == NULL || file == NULL)
        return;

    toml_section_t* currentSection = NULL;
    char* split0 = NULL;
    char* split1 = NULL;

    char line[4096];
    size_t lineLength = 0;
    while (fgets(line, 4096, file) != NULL)
    {
        strtrim(line);
        lineLength = strlen(line);
        if (lineLength == 0)
            continue;

        if (strcmp(line, "[]") == 0)
        {
            currentSection = NULL;
            continue;
        }

        if (line[0] == '[' && line[lineLength-1] == ']')
        {
            memmove(line, &line[1], lineLength-2);
            line[lineLength-2] = 0;
            lineLength -= 2;

            if (toml_file_has_section(config, line))
                currentSection = toml_file_get_section(config, line);
            else
                currentSection = toml_file_add_section(config, line);

            continue;
        }

        split0 = line;
        split1 = strchr(line, '=');
        if (!split1)
            continue;

        split1[0] = 0;
        split1++;

        strtrim(split0);
        strtrim(split1);

        toml_property_t* property = NULL;
        if (currentSection != NULL)
            property = toml_section_add_property(currentSection, split0);
        else
            property = toml_file_add_property(config, split0);

        toml_property_set_value_from_string(property, split1);
    }
}

void toml_file_save(toml_file_t* config, FILE* file)
{
    if (config == NULL || file == NULL)
        return;

    for (size_t i = 0; i < config->propertyCount; i++)
        toml_property_save(&config->properties[i], file);

    if (config->propertyCount != 0)
        fprintf(file, "\n");

    fflush(file);

    for (size_t i = 0; i < config->sectionCount; i++)
        toml_section_save(&config->sections[i], file, NULL);

    fflush(file);
}

toml_property_t* toml_file_get(toml_file_t* config, const char* name)
{
    if (config == NULL || name == NULL || strlen(name) == 0)
        return NULL;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        for (size_t i = 0; i < config->propertyCount; i++)
        {
            if (strcmp(config->properties[i].name, name) != 0)
                continue;

            free(str);
            return &config->properties[i];
        }
        free(str);
        return NULL;
    }

    sectionDivider[0] = 0;
    sectionDivider++;
    if (!toml_file_has_section(config, str))
    {
        free(str);
        return NULL;
    }

    toml_section_t* subSection = toml_file_get_section(config, str);
    toml_property_t* result = toml_section_get(subSection, sectionDivider);
    free(str);
    return result;
}

toml_section_t* toml_file_get_section(toml_file_t* config, const char* name)
{
    if (config == NULL || name == NULL || strlen(name) == 0)
        return NULL;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        for (size_t i = 0; i < config->sectionCount; i++)
        {
            if (strcmp(config->sections[i].name, name) != 0)
                continue;

            free(str);
            return &config->sections[i];
        }
        free(str);
        return NULL;
    }

    sectionDivider[0] = 0;
    sectionDivider++;
    if (!toml_file_has_section(config, str))
    {
        free(str);
        return NULL;
    }

    toml_section_t* subSection = toml_file_get_section(config, str);
    toml_section_t* result = toml_section_get_section(subSection, sectionDivider);
    free(str);
    return result;
}

int toml_file_has_property(toml_file_t* config, const char* name)
{
    if (config == NULL || name == NULL || strlen(name) == 0)
        return 0;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        for (size_t i = 0; i < config->propertyCount; i++)
        {
            if (strcmp(config->properties[i].name, name) != 0)
                continue;

            free(str);
            return 1;
        }
        free(str);
        return 0;
    }

    sectionDivider[0] = 0;
    sectionDivider++;
    if (!toml_file_has_section(config, str))
    {
        free(str);
        return 0;
    }

    toml_section_t* subSection = toml_file_get_section(config, str);
    int result = toml_section_has_property(subSection, sectionDivider);
    free(str);
    return result;
}

int toml_file_has_section(toml_file_t* config, const char* name)
{
    if (config == NULL || name == NULL || strlen(name) == 0)
        return 0;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        for (size_t i = 0; i < config->sectionCount; i++)
        {
            if (strcmp(config->sections[i].name, name) != 0)
                continue;

            free(str);
            return 1;
        }
        free(str);
        return 0;
    }

    sectionDivider[0] = 0;
    sectionDivider++;
    if (!toml_file_has_section(config, str))
    {
        free(str);
        return 0;
    }

    toml_section_t* subSection = toml_file_get_section(config, str);
    int result = toml_section_has_section(subSection, sectionDivider);
    free(str);
    return result;
}

toml_property_t* toml_file_add_property(toml_file_t* config, const char* name)
{
    if (config == NULL || name == NULL || strlen(name) == 0)
        return NULL;

    if (toml_file_has_section(config, name))
        return NULL;

    if (toml_file_has_property(config, name))
        return toml_file_get(config, name);

    toml_property_t propertry = TOML_PROPERTY_EMPTY;
    toml_property_t* propertryPtr = NULL;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        propertry.name = strdup(name);

        config->propertyCount++;
        config->properties = (toml_property_t*)realloc(config->properties, config->propertyCount*sizeof(toml_property_t));
        propertryPtr = &config->properties[config->propertyCount-1];
        memcpy(propertryPtr, &propertry, sizeof(toml_property_t));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        toml_section_t* nextSection = NULL;
        if (!toml_file_has_section(config, str))
            nextSection = toml_file_add_section(config, str);
        else
            nextSection = toml_file_get_section(config, str);

        if (nextSection == NULL)
        {
            free(str);
            return NULL;
        }

        propertryPtr = toml_section_add_property(nextSection, sectionDivider);
    }

    free(str);
    return propertryPtr;
}

toml_section_t* toml_file_add_section(toml_file_t* config, const char* name)
{
    if (config == NULL || name == NULL || strlen(name) == 0)
        return NULL;

    if (toml_file_has_section(config, name))
        return toml_file_get_section(config, name);

    toml_section_t newSection = TOML_SECTION_EMPTY;
    toml_section_t* newSectionPtr = NULL;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        newSection.name = strdup(name);

        config->sectionCount++;
        config->sections = (toml_section_t*)realloc(config->sections, config->sectionCount*sizeof(toml_section_t));
        newSectionPtr = &config->sections[config->sectionCount-1];
        memcpy(newSectionPtr, &newSection, sizeof(toml_section_t));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        toml_section_t* nextSection = NULL;
        if (!toml_file_has_section(config, str))
            nextSection = toml_file_add_section(config, str);
        else
            nextSection = toml_file_get_section(config, str);

        if (nextSection == NULL)
        {
            free(str);
            return NULL;
        }

        newSectionPtr = toml_section_add_section(nextSection, sectionDivider);
    }

    free(str);
    return newSectionPtr;
}

void toml_file_remove_property(toml_file_t* config, const char* name)
{
    if (config == NULL || name == NULL || strlen(name) == 0)
        return;

    if (toml_file_has_section(config, name))
        return;

    if (!toml_file_has_property(config, name))
        return;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        size_t index = 0;
        for (; index < config->propertyCount; index++)
        {
            if (strcmp(name, config->properties[index].name) == 0)
                break;
        }
        toml_property_dispose(&config->properties[index]);
        if (index != config->propertyCount-1)
        {
            toml_property_t tmp = config->properties[config->propertyCount-1];
            config->properties[config->propertyCount-1] = config->properties[index];
            config->properties[index] = tmp;
        }

        config->propertyCount--;
        config->properties = (toml_property_t*)realloc(config->properties, config->propertyCount*sizeof(toml_property_t));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        toml_section_t* nextSection = toml_file_get_section(config, str);
        if (nextSection == NULL)
        {
            free(str);
            return;
        }

        toml_section_remove_property(nextSection, sectionDivider);
    }

    free(str);
}

void toml_file_remove_section(toml_file_t* config, const char* name)
{
    if (config == NULL || name == NULL || strlen(name) == 0)
        return;

    if (!toml_file_has_section(config, name))
        return;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        size_t index = 0;
        for (; index < config->sectionCount; index++)
        {
            if (strcmp(name, config->sections[index].name) == 0)
                break;
        }
        toml_section_dispose(&config->sections[index]);
        if (index != config->sectionCount-1)
        {
            toml_section_t tmp = config->sections[config->sectionCount-1];
            config->sections[config->sectionCount-1] = config->sections[index];
            config->sections[index] = tmp;
        }

        config->sectionCount--;
        config->sections = (toml_section_t*)realloc(config->sections, config->sectionCount*sizeof(toml_section_t));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        toml_section_t* nextSection = toml_file_get_section(config, str);
        if (nextSection == NULL)
        {
            free(str);
            return;
        }

        toml_section_remove_section(nextSection, sectionDivider);
    }

    free(str);
}


#endif

#endif
