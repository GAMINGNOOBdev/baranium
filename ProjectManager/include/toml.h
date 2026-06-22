/**
 * @file toml.h
 * @author GAMINGNOOBdev (https://github.com/GAMINGNOOBdev)
 * @brief A simple toml-like toml file parser/writer utility
 *
 * @note Define TOML_IMPLEMENTATION in ONE!! file where you include this header
 *
 * @copyright Copyright (c) GAMINGNOOBdev
 */
#ifndef __TOML_H_
#define __TOML_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

///////////////////
///             ///
///   Defines   ///
///             ///
///////////////////

#define TOML_PROPERTY_INIT(_name, _type, _value, _arrayLength, _parent) (toml_property){.name=_name, .type=_type, .value=_value, .arrayLength=_arrayLength, .parent=_parent}
#define TOML_PROPERTY_EMPTY TOML_PROPERTY_INIT(0, 0, {0}, 0, 0)
#define TOML_SECTION_EMPTY (toml_section){.name=(const char*)0, .parent=(toml_section*)0, .sections=(toml_section*)0, .sectionCount=0, .properties=(toml_property*)0, .propertyCount=0}

#define FOREACH_PROPERTY(name, containerptr, handle) for (size_t i##name = 0; containerptr && i##name < containerptr->propertyCount; i##name++) { toml_property* name = &containerptr->properties[i##name]; handle }
#define FOREACH_SECTION(name, containerptr, handle) for (size_t i##name = 0; containerptr && i##name < containerptr->sectionCount; i##name++) { toml_section* name = &containerptr->sections[i##name]; handle }

#define TOML_PROPERTY_TYPE_UNKNOWN  ((int)0)
#define TOML_PROPERTY_TYPE_STRING   ((int)1)
#define TOML_PROPERTY_TYPE_FLOAT    ((int)2)
#define TOML_PROPERTY_TYPE_BOOL     ((int)3)
#define TOML_PROPERTY_TYPE_INT      ((int)4)
#define TOML_PROPERTY_TYPE_ARRAY    ((int)5)

struct toml_section;

//////////////////////////
///                    ///
///   util functions   ///
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
 * @brief toml property type as string
 *
 * @param type Type
 * @returns Type name as string
 */
const char* toml_property_value_type_as_string(int type);

/////////////////////////
///                   ///
///   toml property   ///
///                   ///
/////////////////////////

typedef struct toml_property
{
    const char* name;
    int type;
    union {
        struct toml_property* arrayValue;
        const char* stringValue;
        float floatValue;
        int64_t intValue;
        uint8_t boolValue;
    } value;
    int arrayLength;
    char quoteCharacter;

    struct toml_section* parent;
} toml_property;

/**
 * @brief Dispose a toml property
 *
 * @param property toml property
 */
void toml_property_dispose(toml_property* property);

/**
 * @brief Duplicates all contents of a property
 *
 * @note Returned property has to be disposed with `toml_property_dispose()`
 *
 * @param property toml property
 * @returns The property with all duplicated contents
 */
toml_property toml_property_duplicate(toml_property* property);

/**
 * @brief Get full toml property name
 *
 * @note Returned string has to be manually cleaned up by the used using `free()`
 *
 * @param property toml property
 * @returns Property name
 */
const char* toml_property_get_full_name(toml_property* property);

/**
 * @brief Add an array element to this property (if it is of the array type)
 *
 * @param property toml property
 * @param other Other property
 */
void toml_property_array_add(toml_property* property, toml_property* other);

/**
 * @brief Insert an array element to this property at the given index (pushes values at the index towards the end) (if it is of the array type)
 *
 * @param property toml property
 * @param index Index
 * @param other Other property
 */
void toml_property_array_insert_at(toml_property* property, int index, toml_property* other);

/**
 * @brief Remove an array element from this property (if it is of the array type)
 *
 * @param property toml property
 * @param index Index
 */
void toml_property_array_remove(toml_property* property, int index);

/**
 * @brief Set toml property value from a string
 *
 * @note `value` should be modifyable and will stay modified after use
 *
 * @param property toml property
 * @param value Value string
 */
void toml_property_set_value_from_string(toml_property* property, const char* value);

/**
 * @brief Set toml property value
 *
 * @note `value` should be modifyable and will stay modified after use
 *
 * @param property toml property
 * @param value Value
 */
void toml_property_set_string(toml_property* property, const char* value);

/**
 * @brief Set toml property value
 *
 * @param property toml property
 * @param value Value
 */
void toml_property_set_float(toml_property* property, float value);

/**
 * @brief Set toml property value
 *
 * @param property toml property
 * @param value Value
 */
void toml_property_set_bool(toml_property* property, uint8_t value);

/**
 * @brief Set toml property value
 *
 * @param property toml property
 * @param value Value
 */
void toml_property_set_int(toml_property* property, int64_t value);

/**
 * @brief Set toml property value
 *
 * @param property toml property
 * @param len Array length
 * @param array Array of values
 */
void toml_property_set_array(toml_property* property, int len, toml_property array[]);

/**
 * @brief Save property to a file
 *
 * @param property toml property
 * @param file Output file
 */
void toml_property_save(toml_property* property, FILE* file);

/**
 * @brief Save property to a file
 *
 * @note MAKE SURE THE BUFFER IS BIG ENOUGH
 *
 * @param property toml property
 * @param buf Output buffer
 * @param maxlen Max Buffer length
 */
void toml_property_as_str(toml_property* property, char* buf, size_t maxlen);

/**
 * @brief Compare two properties to each other
 *
 * @param a First property
 * @param b Second property
 * @returns Comparison result, 0 if equal
 */
int toml_property_cmp(toml_property* a, toml_property* b);

////////////////////////
///                  ///
///   toml section   ///
///                  ///
////////////////////////

typedef struct toml_section
{
    const char* name;
    struct toml_section* parent;

    struct toml_section* sections;
    size_t sectionCount;

    struct toml_property* properties;
    size_t propertyCount;
} toml_section;

/**
 * @brief Dispose a toml section
 *
 * @param section toml section
 */
void toml_section_dispose(toml_section* section);

/**
 * @brief Invalidate parents of a section
 */
void toml_section_invalidate_parents(toml_section* parent, toml_section* section);

/**
 * @brief Get full toml section name
 *
 * @note Returned string has to be manually cleaned up by the used using `free()`
 *
 * @param section toml section
 * @returns Section name
 */
const char* toml_section_get_full_name(toml_section* section);

/**
 * @brief Save toml section to a file
 *
 * @param section toml section
 * @param file Output file
 * @param prefix Section name prefix
 */
void toml_section_save(toml_section* section, FILE* file, const char* prefix);

/**
 * @brief Get a property inside this section
 *
 * @param section toml section
 * @param name Property name
 * @returns The desired toml property
 */
toml_property* toml_section_get(toml_section* section, const char* name);

/**
 * @brief Get a section inside this section
 *
 * @param section toml section
 * @param name Section name
 * @returns The desired toml section
 */
toml_section* toml_section_get_section(toml_section* section, const char* name);

/**
 * @brief Get whether a property exists in this section
 *
 * @param section toml section
 * @param name Property name
 * @returns 0 if no, 1 if yes
 */
int toml_section_has_property(toml_section* section, const char* name);

/**
 * @brief Get whether a section exists in this section
 *
 * @param section toml section
 * @param name Section name
 * @returns 0 if no, 1 if yes
 */
int toml_section_has_section(toml_section* section, const char* name);

/**
 * @brief Add a property to this section
 *
 * @param section toml section
 * @param name Property name
 * @returns The newly created property
 */
toml_property* toml_section_add_property(toml_section* section, const char* name);

/**
 * @brief Add a section to this section
 *
 * @param section toml section
 * @param name Section name
 * @returns The newly created section
 */
toml_section* toml_section_add_section(toml_section* section, const char* name);

/**
 * @brief Remove a property from this section
 *
 * @param section toml section
 * @param name Property name
 */
void toml_section_remove_property(toml_section* section, const char* name);

/**
 * @brief Remove a section from this section
 *
 * @param section toml section
 * @param name Section name
 */
void toml_section_remove_section(toml_section* section, const char* name);

/**
 * @brief Compare two sections to each other
 *
 * @param a First section
 * @param b Second section
 * @returns Comparison result, 0 if equal
 */
int toml_section_cmp(toml_section* a, toml_section* b);

/////////////////////
///               ///
///   toml file   ///
///               ///
/////////////////////

/**
 * @brief Open and parse a toml file
 *
 * @param file Input file
 * @returns parsed toml file
 */
toml_section toml_file_open(FILE* file);

#ifdef __cplusplus
}
#endif

#ifdef TOML_IMPLEMENTATION

#include <stdint.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

//////////////////////////
///                    ///
///   util functions   ///
///                    ///
//////////////////////////

// free a block of memory IF not null
#define safefree(block) if(block) free((void*)block)

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
        return TOML_PROPERTY_TYPE_UNKNOWN;

    if (is_bool(value))
        return TOML_PROPERTY_TYPE_BOOL;

    if (is_integer(value))
        return TOML_PROPERTY_TYPE_INT;

    if (is_float(value))
        return TOML_PROPERTY_TYPE_FLOAT;

    if (value[0] == '[' && value[valueLen-1] == ']')
        return TOML_PROPERTY_TYPE_ARRAY;

    if ((value[0] == '\'' || value[0] == '"') && (value[valueLen-1] == '\'' || value[valueLen-1] == '"'))
        return TOML_PROPERTY_TYPE_STRING;

    return TOML_PROPERTY_TYPE_STRING;
}

const char* toml_property_value_type_as_string(int type)
{
    if (type == TOML_PROPERTY_TYPE_STRING)
        return "STRING";

    if (type == TOML_PROPERTY_TYPE_FLOAT)
        return "FLOAT";

    if (type == TOML_PROPERTY_TYPE_BOOL)
        return "BOOL";

    if (type == TOML_PROPERTY_TYPE_INT)
        return "INT";

    if (type == TOML_PROPERTY_TYPE_ARRAY)
        return "ARRAY";

    return "UNKNOWN";
}

//////////////////////////
///                    ///
///   toml file dict   ///
///                    ///
//////////////////////////

#define TOML_FILE_BUFFER_SIZE 0x10

typedef const char* string;

typedef struct
{
    size_t bufferSize;
    string* data;
    size_t count;
} toml_dict;

void toml_dict_ensure_buffer(toml_dict* dict)
{
    if (dict->count < dict->bufferSize)
        return;

    dict->bufferSize += TOML_FILE_BUFFER_SIZE;
    dict->data = (string*)realloc(dict->data, sizeof(string)*dict->bufferSize);
}

int toml_dict_add(toml_dict* dict, string element)
{
    if (!dict || !element)
        return -1;

    for (size_t i = 0; i < dict->count; i++)
    {
        if (strcmp(element, dict->data[i]) == 0)
            return i;
    }

    dict->count++;
    toml_dict_ensure_buffer(dict);
    dict->data[dict->count-1] = strdup(element);
    return dict->count-1;
}

void toml_dict_dispose(toml_dict* dict)
{
    if (!dict || !dict->data)
        return;

    for (size_t i = 0; i < dict->count; i++)
        safefree(dict->data[i]);

    safefree(dict->data);
    memset(dict, 0, sizeof(toml_dict));
}

///////////////////////////////
///                         ///
///   toml file tokenizer   ///
///                         ///
///////////////////////////////

#define TOML_TOKEN_TYPE_DEFAULT 0
#define TOML_TOKEN_TYPE_KEY     1
#define TOML_TOKEN_TYPE_VALUE   2

typedef struct
{
    char c;
    int type;
} toml_special_char;

const toml_special_char TOML_SPECIAL_CHARS[] = {
    {'[', TOML_TOKEN_TYPE_DEFAULT},
    {']', TOML_TOKEN_TYPE_DEFAULT},
    {'=', TOML_TOKEN_TYPE_DEFAULT},
    {',', TOML_TOKEN_TYPE_DEFAULT},
    {'{', TOML_TOKEN_TYPE_DEFAULT},
    {'}', TOML_TOKEN_TYPE_DEFAULT},
    {'\0', -1}
};

int toml_is_special_char(char c)
{
    for (int i = 0; TOML_SPECIAL_CHARS[i].c != '\0'; i++)
        if (TOML_SPECIAL_CHARS[i].c == c)
            return TOML_SPECIAL_CHARS[i].type;

    return -1;
}

typedef struct
{
    toml_dict dict;
    uint64_t* data;
    size_t bufferSize;
    size_t count;
    struct {
        uint8_t lastQuoteCharacter;
        size_t bufferIndex;
        char buffer[4096];
    } tokenizer;
} toml_file_tokenizer;

#define TOML_TOKEN(idx, type) ((uint64_t)idx << 32 | (uint64_t)type)
#define TOML_TOKEN_SPECIAL(idx, type, c) ((uint64_t)idx << 32 | (uint64_t)c << 16 | (uint64_t)type)
#define TOML_TOKEN_INDEX(num) ((int32_t)(num >> 32))
#define TOML_TOKEN_TYPE(num) ((int32_t)num & 0xFFFF)
#define TOML_TOKEN_CHAR(num) ((uint32_t)num & 0xFF0000) >> 16

void toml_file_tokenizer_ensure_buffer(toml_file_tokenizer* file)
{
    if (file->count < file->bufferSize)
        return;

    file->bufferSize += TOML_FILE_BUFFER_SIZE;
    file->data = (uint64_t*)realloc(file->data, sizeof(uint64_t)*file->bufferSize);
}

void toml_file_tokenizer_add(toml_file_tokenizer* file, int dictIdx, int type, char c)
{
    if (!file)
        return;

    file->count++;
    toml_file_tokenizer_ensure_buffer(file);
    file->data[file->count-1] = TOML_TOKEN_SPECIAL(dictIdx, type, c);
}

void toml_file_tokenizer_pop(toml_file_tokenizer* file)
{
    if (!file)
        return;

    file->count--;
}

void toml_file_tokenizer_dispose(toml_file_tokenizer* file)
{
    if (!file)
        return;

    safefree(file->data);
    toml_dict_dispose(&file->dict);
    memset(file, 0, sizeof(toml_file_tokenizer));
}

void toml_file_tokenizer_tokenize_char(toml_file_tokenizer* file, char c)
{
    char buf[2] = {c, '\0'};
    int dictIdx = toml_dict_add(&file->dict, buf);
    int type = toml_is_special_char(c);
    toml_file_tokenizer_add(file, dictIdx, type != -1 ? type : TOML_TOKEN_TYPE_DEFAULT, 0);
}

void toml_file_tokenizer_tokenize_buffer(toml_file_tokenizer* file, string line, uint8_t isString, char usedQuote)
{
    if (!line || *line == '\0') return;

    int idx = toml_dict_add(&file->dict, line);
    int type = isString ? TOML_TOKEN_TYPE_VALUE : TOML_TOKEN_TYPE_KEY;
    toml_file_tokenizer_add(file, idx, type, usedQuote);
}

void toml_file_tokenizer_tokenize_line(toml_file_tokenizer* file, string line, size_t len)
{
    if (!file || !line || !len)
        return;

    for (size_t i = 0; i < len; i++)
    {
        char c = line[i];
        if (c == '"' || c == '\'')
        {
            if (file->tokenizer.lastQuoteCharacter == c)
            {
                file->tokenizer.buffer[file->tokenizer.bufferIndex] = '\0';
                toml_file_tokenizer_tokenize_buffer(file, file->tokenizer.buffer, 1, c);
                file->tokenizer.bufferIndex = 0;
                file->tokenizer.lastQuoteCharacter = 0;
                continue;
            }
            else if (file->tokenizer.lastQuoteCharacter == 0)
            {
                file->tokenizer.lastQuoteCharacter = c;
                continue;
            }
            if (file->tokenizer.bufferIndex < sizeof(file->tokenizer.buffer) - 1)
                file->tokenizer.buffer[file->tokenizer.bufferIndex++] = c;

            continue;
        }

        if (file->tokenizer.lastQuoteCharacter != 0)
        {
            if (file->tokenizer.bufferIndex < sizeof(file->tokenizer.buffer) - 1)
                file->tokenizer.buffer[file->tokenizer.bufferIndex++] = c;
            continue;
        }

        if (c == '#')
            break;

        if (toml_is_special_char(c) != -1)
        {
            if (file->tokenizer.bufferIndex > 0)
            {
                file->tokenizer.buffer[file->tokenizer.bufferIndex] = '\0';
                toml_file_tokenizer_tokenize_buffer(file, file->tokenizer.buffer, 0, 0);
                file->tokenizer.bufferIndex = 0;
            }
            toml_file_tokenizer_tokenize_char(file, c);
            continue;
        }

        if (isspace(c))
        {
            if (file->tokenizer.bufferIndex > 0)
            {
                file->tokenizer.buffer[file->tokenizer.bufferIndex] = '\0';
                toml_file_tokenizer_tokenize_buffer(file, file->tokenizer.buffer, 0, 0);
                file->tokenizer.bufferIndex = 0;
            }
            continue;
        }

        if (file->tokenizer.bufferIndex < sizeof(file->tokenizer.buffer) - 1)
            file->tokenizer.buffer[file->tokenizer.bufferIndex++] = c;
    }

    if (file->tokenizer.lastQuoteCharacter != 0)
        return;

    if (file->tokenizer.bufferIndex > 0)
    {
        file->tokenizer.buffer[file->tokenizer.bufferIndex] = '\0';
        toml_file_tokenizer_tokenize_buffer(file, file->tokenizer.buffer, 0, 0);
        file->tokenizer.bufferIndex = 0;
    }
}

size_t toml_file_tokenizer_resolve_path(toml_file_tokenizer* file, size_t tokenIdx, char end, char* output)
{
    output[0] = '\0';
    while (tokenIdx < file->count)
    {
        uint64_t token = file->data[tokenIdx];
        string s = file->dict.data[TOML_TOKEN_INDEX(token)];

        if (TOML_TOKEN_TYPE(token) == TOML_TOKEN_TYPE_DEFAULT && s[0] == end)
            break;

        strcat(output, s);
        tokenIdx++;
    }
    return tokenIdx;
}

/////////////////////////
///                   ///
///   toml property   ///
///                   ///
/////////////////////////

void toml_property_dispose_value(toml_property* property)
{
    if (!property)
        return;

    for (int i = 0; i < property->arrayLength; i++)
        toml_property_dispose(&property->value.arrayValue[i]);

    if (property->type == TOML_PROPERTY_TYPE_STRING || property->type == TOML_PROPERTY_TYPE_UNKNOWN)
        safefree((void*)property->value.stringValue);

    if (property->type == TOML_PROPERTY_TYPE_ARRAY)
        safefree((void*)property->value.arrayValue);

    property->arrayLength = 0;
    memset(&property->value, 0, sizeof(property->value));
}

void toml_property_dispose(toml_property* property)
{
    if (property == NULL)
        return;

    safefree((void*)property->name);

    toml_property_dispose_value(property);

    memset(property, 0, sizeof(toml_property));
}

toml_property toml_property_duplicate(toml_property* property)
{
    toml_property result = TOML_PROPERTY_EMPTY;
    if (property == NULL)
        return result;

    if (property->name)
        result.name = strdup(property->name);
    result.arrayLength = property->arrayLength;
    result.type = property->type;
    result.parent = property->parent;
    result.value = property->value;

    if (result.type == TOML_PROPERTY_TYPE_ARRAY)
    {
        result.value.arrayValue = (toml_property*)malloc(sizeof(toml_property)*result.arrayLength);
        for (int i = 0; i < result.arrayLength; i++)
            result.value.arrayValue[i] = toml_property_duplicate(&property->value.arrayValue[i]);
        return result;
    }

    if (result.type == TOML_PROPERTY_TYPE_UNKNOWN || result.type == TOML_PROPERTY_TYPE_STRING)
    {
        result.value.stringValue = strdup(result.value.stringValue);
        return result;
    }

    return result;
}

const char* toml_property_get_full_name(toml_property* property)
{
    if (property == NULL || property->name == NULL)
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

void toml_property_array_add(toml_property* property, toml_property* other)
{
    if (property == NULL || other == NULL)
        return;

    if (property->type != TOML_PROPERTY_TYPE_ARRAY)
        return;

    property->arrayLength++;
    property->value.arrayValue = (toml_property*)realloc(property->value.arrayValue, sizeof(toml_property)*property->arrayLength);
    property->value.arrayValue[property->arrayLength-1] = toml_property_duplicate(other);
}

void toml_property_array_insert_at(toml_property* property, int index, toml_property* other)
{
    if (property == NULL || index < 0 || other == NULL)
        return;

    if (property->type != TOML_PROPERTY_TYPE_ARRAY)
        return;

    if ((property->arrayLength == 0 && index == 0) || index == -1)
    {
        toml_property_array_add(property, other);
        return;
    }

    if (index >= property->arrayLength)
        return;

    property->arrayLength++;
    property->value.arrayValue = (toml_property*)realloc(property->value.arrayValue, sizeof(toml_property)*property->arrayLength);
    memmove(&property->value.arrayValue[index+1], &property->value.arrayValue[index], sizeof(toml_property)*(property->arrayLength-index-1));
    property->value.arrayValue[index] = toml_property_duplicate(other);
}

void toml_property_array_remove(toml_property* property, int index)
{
    if (property == NULL)
        return;

    if (property->type != TOML_PROPERTY_TYPE_ARRAY)
        return;

    if (property->arrayLength == 0)
        return;

    if (index == -1 || index == property->arrayLength-1)
    {
        toml_property_dispose(&property->value.arrayValue[property->arrayLength-1]);
        property->arrayLength--;
        property->value.arrayValue = (toml_property*)realloc(property->value.arrayValue, sizeof(toml_property)*property->arrayLength);
        return;
    }
    else if (index == 0)
    {
        toml_property tmp = property->value.arrayValue[0];
        toml_property_dispose(&tmp);
        memmove(property->value.arrayValue, &property->value.arrayValue[1], sizeof(toml_property)*property->arrayLength-1);
        property->arrayLength--;
        property->value.arrayValue = (toml_property*)realloc(property->value.arrayValue, sizeof(toml_property)*property->arrayLength);
        return;
    }
    else if (index >= property->arrayLength)
        return;

    toml_property tmp = property->value.arrayValue[index];
    toml_property_dispose(&tmp);
    memmove(&property->value.arrayValue[index], &property->value.arrayValue[index+1], sizeof(toml_property)*(property->arrayLength-index-1));
    property->arrayLength--;
    property->value.arrayValue = (toml_property*)realloc(property->value.arrayValue, sizeof(toml_property)*property->arrayLength);
}

void toml_property_parse_primitive(toml_property* property, const char* valStr)
{
    char* tmp;
    property->type = toml_estimate_value_type(valStr);

    switch (property->type)
    {
        case TOML_PROPERTY_TYPE_INT:
            property->value.intValue = strtol(valStr, &tmp, 10);
            break;
        case TOML_PROPERTY_TYPE_FLOAT:
            property->value.floatValue = strtof(valStr, &tmp);
            break;
        case TOML_PROPERTY_TYPE_BOOL:
            property->value.boolValue = (strcmp(valStr, "true") == 0 || strcmp(valStr, "True") == 0);
            break;
        case TOML_PROPERTY_TYPE_STRING:
        default:
            property->value.stringValue = strdup(valStr);
            break;
    }
}

size_t toml_property_parse_value(toml_file_tokenizer* tokenizer, size_t tokenIdx, toml_property* property)
{
    if (tokenIdx >= tokenizer->count) return tokenIdx;

    uint64_t token = tokenizer->data[tokenIdx];
    uint8_t tokenType = TOML_TOKEN_TYPE(token);
    string valStr = tokenizer->dict.data[TOML_TOKEN_INDEX(token)];

    if (tokenType != TOML_TOKEN_TYPE_DEFAULT || valStr[0] != '[')
    {
        char tokenChar = TOML_TOKEN_CHAR(token);
        if (tokenChar)
        {
            toml_property_set_string(property, valStr);
            property->quoteCharacter = tokenChar;
            return tokenIdx + 1;
        }
        toml_property_parse_primitive(property, valStr);
        if (property->type == TOML_PROPERTY_TYPE_STRING) // failsafe
        {
            tokenChar = '"';
            property->quoteCharacter = tokenChar;
        }
        return tokenIdx + 1;
    }
    property->type = TOML_PROPERTY_TYPE_ARRAY;
    tokenIdx++;

    while (tokenIdx < tokenizer->count)
    {
        token = tokenizer->data[tokenIdx];
        tokenType = TOML_TOKEN_TYPE(token);
        string currentStr = tokenizer->dict.data[TOML_TOKEN_INDEX(token)];

        if (tokenType == TOML_TOKEN_TYPE_DEFAULT && currentStr[0] == ']')
            return tokenIdx + 1;

        if (tokenType == TOML_TOKEN_TYPE_DEFAULT && currentStr[0] == ',')
        {
            tokenIdx++;
            continue;
        }

        toml_property element = TOML_PROPERTY_EMPTY;
        tokenIdx = toml_property_parse_value(tokenizer, tokenIdx, &element);

        toml_property_array_add(property, &element);
        toml_property_dispose(&element);
    }
    return tokenIdx;
}

void toml_property_set_value_from_string(toml_property* property, const char* string)
{
    if (property == NULL || string == NULL)
        return;

    toml_file_tokenizer tokenizer = {};
    toml_file_tokenizer_tokenize_line(&tokenizer, string, strlen(string));
    if (tokenizer.count == 0)
    {
        toml_file_tokenizer_dispose(&tokenizer);
        return;
    }

    toml_property_dispose_value(property);
    toml_property_parse_value(&tokenizer, 0, property);
    toml_file_tokenizer_dispose(&tokenizer);
}

void toml_property_set_string(toml_property* property, const char* value)
{
    if (property == NULL)
        return;
    toml_property_dispose_value(property);

    property->arrayLength = 0;
    property->type = TOML_PROPERTY_TYPE_STRING;
    property->quoteCharacter = '"';
    if (value == NULL)
    {
        property->value.stringValue = NULL;
        return;
    }

    property->value.stringValue = strdup(value);
}

void toml_property_set_float(toml_property* property, float value)
{
    if (property == NULL)
        return;
    toml_property_dispose_value(property);

    property->arrayLength = 0;
    property->type = TOML_PROPERTY_TYPE_FLOAT;
    property->value.floatValue = value;
}

void toml_property_set_bool(toml_property* property, uint8_t value)
{
    if (property == NULL)
        return;
    toml_property_dispose_value(property);

    property->arrayLength = 0;
    property->type = TOML_PROPERTY_TYPE_BOOL;
    property->value.boolValue = value;
}

void toml_property_set_int(toml_property* property, int64_t value)
{
    if (property == NULL)
        return;
    toml_property_dispose_value(property);

    property->arrayLength = 0;
    property->type = TOML_PROPERTY_TYPE_INT;
    property->value.intValue = value;
}

void toml_property_set_array(toml_property* property, int len, toml_property array[])
{
    if (property == NULL)
        return;
    toml_property_dispose_value(property);

    property->arrayLength = len;
    property->type = TOML_PROPERTY_TYPE_ARRAY;
    property->value.arrayValue = (toml_property*)malloc(sizeof(toml_property)*len);
    for (int i = 0; i < len; i++)
        property->value.arrayValue[i] = toml_property_duplicate(&array[i]);
}

void toml_property_save_value(toml_property* property, FILE* file)
{
    if (property == NULL || file == NULL)
        return;

    switch (property->type)
    {
    case TOML_PROPERTY_TYPE_INT:
    {
        fprintf(file, "%ld", property->value.intValue);
        break;
    }

    case TOML_PROPERTY_TYPE_FLOAT:
    {
        fprintf(file, "%f", property->value.floatValue);
        break;
    }

    case TOML_PROPERTY_TYPE_BOOL:
    {
        fprintf(file, "%s", property->value.boolValue ? "true" : "false");
        break;
    }

    case TOML_PROPERTY_TYPE_ARRAY:
    {
        fprintf(file, "[ ");
        for (int i = 0; i < property->arrayLength; i++)
        {
            toml_property* tmp = &property->value.arrayValue[i];
            toml_property_save_value(tmp, file);
            if (i != property->arrayLength - 1)
                fprintf(file, ", ");
        }
        fprintf(file, " ]");
        break;
    }

    case TOML_PROPERTY_TYPE_UNKNOWN:
    case TOML_PROPERTY_TYPE_STRING:
    {
        if (!property->quoteCharacter)
            property->quoteCharacter = '"';

        string value = property->value.stringValue;
        if (value == NULL)
        {
            fprintf(file, "''");
            break;
        }
        size_t len = strlen(value);
        if (value[len-1] == '\n')
            memset((void*)&value[len-1], 0, 1);
        fprintf(file, "%c%s%c", property->quoteCharacter, value, property->quoteCharacter);
        break;
    }
    }
}

void toml_property_save(toml_property* property, FILE* file)
{
    if (property == NULL || file == NULL)
        return;

    fprintf(file, "%s = ", property->name);
    toml_property_save_value(property, file);
    fprintf(file, "\n");
    fflush(file);
}

void toml_property_as_str(toml_property* property, char* buf, size_t maxlen)
{
    if (property == NULL || buf == NULL || !maxlen)
        return;

    switch (property->type)
    {
    case TOML_PROPERTY_TYPE_INT:
    {
        snprintf(buf, maxlen, "%ld", property->value.intValue);
        break;
    }

    case TOML_PROPERTY_TYPE_FLOAT:
    {
        snprintf(buf, maxlen, "%f", property->value.floatValue);
        break;
    }

    case TOML_PROPERTY_TYPE_BOOL:
    {
        snprintf(buf, maxlen, "%s", property->value.boolValue ? "true" : "false");
        break;
    }

    case TOML_PROPERTY_TYPE_ARRAY:
    {
        char buffer[0x1000] = {0};
        char buffer2[0x1000] = {0};
        for (int i = 0; i < property->arrayLength; i++)
        {
            toml_property* tmp = &property->value.arrayValue[i];
            toml_property_as_str(tmp, buffer2, 0x1000);
            strcat(buffer, buffer2);
            if (i != property->arrayLength - 1)
                strcat(buffer, ", ");
        }
        snprintf(buf, maxlen, "[ %s ]", buffer);
        break;
    }

    case TOML_PROPERTY_TYPE_UNKNOWN:
    case TOML_PROPERTY_TYPE_STRING:
    {
        if (!property->quoteCharacter)
            property->quoteCharacter = '"';

        string value = property->value.stringValue;
        if (value == NULL)
        {
            snprintf(buf, maxlen, "''");
            break;
        }
        size_t len = strlen(value);
        if (value[len-1] == '\n')
            memset((void*)&value[len-1], 0, 1);
        snprintf(buf, maxlen, "%c%s%c", property->quoteCharacter, value, property->quoteCharacter);
        break;
    }
    }
}

int toml_property_cmp(toml_property* a, toml_property* b)
{
    if (a == NULL || b == NULL)
        return -1;

    if (a->type != b->type)
        return 1;

    return strcmp(a->name, b->name);
}

////////////////////////
///                  ///
///   toml section   ///
///                  ///
////////////////////////

void toml_section_dispose(toml_section* section)
{
    if (section == NULL)
        return;

    for (size_t i = 0; i < section->sectionCount; i++)
        toml_section_dispose(&section->sections[i]);

    for (size_t i = 0; i < section->propertyCount; i++)
        toml_property_dispose(&section->properties[i]);

    safefree((void*)section->name);
    safefree((void*)section->properties);
    safefree((void*)section->sections);

    memset(section, 0, sizeof(toml_section));
}

void toml_section_invalidate_parents(toml_section* parent, toml_section* section)
{
    section->parent = parent;
    for (size_t i = 0; i < section->sectionCount; i++)
        toml_section_invalidate_parents(section, &section->sections[i]);

    for (size_t i = 0; i < section->propertyCount; i++)
        section->properties[i].parent = section;
}

const char* toml_section_get_full_name(toml_section* section)
{
    if (section == NULL || section->name == NULL)
        return NULL;

    if (section->parent == NULL)
        return strdup(section->name);

    char* parentSectionName = (char*)toml_section_get_full_name(section->parent);
    if (parentSectionName == NULL)
        return strdup(section->name);

    char* parent = strdup(parentSectionName);
    parent = (char*)realloc(parent, strlen(parent) + strlen(section->name) + 2);
    free(parentSectionName);
    parent = strcat(parent, ".");
    parent = strcat(parent, section->name);
    return parent;
}

void toml_section_save(toml_section* section, FILE* file, const char* prefix)
{
    if (section == NULL || file == NULL)
        return;

    if (section->name && section->propertyCount)
    {
        fprintf(file, "[");
        if (prefix != NULL)
            fprintf(file, "%s.", prefix);
        fprintf(file, "%s]\n", section->name);
    }

    for (size_t i = 0; i < section->propertyCount; i++)
        toml_property_save(&section->properties[i], file);

    if (section->propertyCount)
        fprintf(file, "\n");

    if (section->sectionCount == 0)
    {
        fflush(file);
        return;
    }

    char* newSectionPrefix = NULL;
    if (prefix != NULL)
    {
        newSectionPrefix = strdup(prefix);
        newSectionPrefix = (char*)realloc(newSectionPrefix, strlen(prefix)+2);
        newSectionPrefix = strcat(newSectionPrefix, ".");
    }
    if (section->name)
    {
        if (newSectionPrefix)
        {
            newSectionPrefix = (char*)realloc(newSectionPrefix, strlen(newSectionPrefix) + strlen(section->name) + 1);
            newSectionPrefix = strcat(newSectionPrefix, section->name);
        }
        else
            newSectionPrefix = strdup(section->name);
    }

    for (size_t i = 0; i < section->sectionCount; i++)
        toml_section_save(&section->sections[i], file, newSectionPrefix);

    safefree(newSectionPrefix);

    fflush(file);
}

toml_property* toml_section_get(toml_section* section, const char* name)
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

    toml_section* subSection = toml_section_get_section(section, str);
    toml_property* result = toml_section_get(subSection, sectionDivider);
    free(str);
    return result;
}

toml_section* toml_section_get_section(toml_section* section, const char* name)
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

    toml_section* subSection = toml_section_get_section(section, str);
    toml_section* result = toml_section_get_section(subSection, sectionDivider);
    free(str);
    return result;
}

int toml_section_has_property(toml_section* section, const char* name)
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
    if (!toml_section_has_section(section, str))
    {
        free(str);
        return 0;
    }

    toml_section* subSection = toml_section_get_section(section, str);
    int result = toml_section_has_property(subSection, sectionDivider);
    free(str);
    return result;
}

int toml_section_has_section(toml_section* section, const char* name)
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

    toml_section* subSection = toml_section_get_section(section, str);
    int result = toml_section_has_section(subSection, sectionDivider);
    free(str);
    return result;
}

toml_property* toml_section_add_property(toml_section* section, const char* name)
{
    if (section == NULL || name == NULL || strlen(name) == 0)
        return NULL;

    if (toml_section_has_section(section, name))
        return NULL;

    if (toml_section_has_property(section, name))
        return toml_section_get(section, name);

    toml_property propertry = TOML_PROPERTY_EMPTY;
    toml_property* propertryPtr = NULL;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        propertry.name = strdup(name);
        propertry.parent = section;

        section->propertyCount++;
        section->properties = (toml_property*)realloc(section->properties, section->propertyCount*sizeof(toml_property));
        propertryPtr = &section->properties[section->propertyCount-1];
        memcpy(propertryPtr, &propertry, sizeof(toml_property));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        toml_section* nextSection = toml_section_add_section(section, str);
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

toml_section* toml_section_add_section(toml_section* section, const char* name)
{
    if (section == NULL || name == NULL || strlen(name) == 0)
        return NULL;

    if (toml_section_has_section(section, name))
        return toml_section_get_section(section, name);

    toml_section newSection = TOML_SECTION_EMPTY;
    toml_section* newSectionPtr = NULL;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        newSection.name = strdup(name);
        newSection.parent = section;

        section->sectionCount++;
        section->sections = (toml_section*)realloc(section->sections, section->sectionCount*sizeof(toml_section));
        newSectionPtr = &section->sections[section->sectionCount-1];
        memcpy(newSectionPtr, &newSection, sizeof(toml_section));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;

        toml_section* nextSection = toml_section_add_section(section, str);
        if (nextSection == NULL)
        {
            free(str);
            return NULL;
        }

        newSectionPtr = toml_section_add_section(nextSection, sectionDivider);
    }

    toml_section_invalidate_parents(section->parent, section);

    free(str);
    return newSectionPtr;
}

void toml_section_remove_property(toml_section* section, const char* name)
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
            toml_property tmp = section->properties[section->propertyCount-1];
            section->properties[section->propertyCount-1] = section->properties[index];
            section->properties[index] = tmp;
        }

        section->propertyCount--;
        section->properties = (toml_property*)realloc(section->properties, section->propertyCount*sizeof(toml_property));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        toml_section* nextSection = toml_section_get_section(section, str);
        if (nextSection == NULL)
        {
            free(str);
            return;
        }

        toml_section_remove_property(nextSection, sectionDivider);
    }

    free(str);
}

void toml_section_remove_section(toml_section* section, const char* name)
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
            toml_section tmp = section->sections[section->sectionCount-1];
            section->sections[section->sectionCount-1] = section->sections[index];
            section->sections[index] = tmp;
        }

        section->sectionCount--;
        section->sections = (toml_section*)realloc(section->sections, section->sectionCount*sizeof(toml_section));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        toml_section* nextSection = toml_section_get_section(section, str);
        if (nextSection == NULL)
        {
            free(str);
            return;
        }

        toml_section_remove_section(nextSection, sectionDivider);
    }

    free(str);
}

int toml_section_cmp(toml_section* a, toml_section* b)
{
    if (a == NULL || b == NULL)
        return -1;

    if (a->propertyCount != b->propertyCount)
        return a->propertyCount - b->propertyCount;

    if (a->sectionCount != b->sectionCount)
        return a->sectionCount - b->sectionCount;

    return strcmp(a->name, b->name);
}

////////////////////////////
///                      ///
///   toml file parser   ///
///                      ///
////////////////////////////

toml_section toml_file_open(FILE* file)
{
    if (file == NULL)
        return TOML_SECTION_EMPTY;

    toml_file_tokenizer tokenizer = {};

    char line[4096];
    size_t lineLength = 0;
    while (fgets(line, 4096, file) != NULL)
    {
        lineLength = strlen(line);
        if (lineLength == 0)
            continue;

        toml_file_tokenizer_tokenize_line(&tokenizer, line, lineLength);
    }

    toml_section root = TOML_SECTION_EMPTY;
    toml_section* currentSection = &root;

    for (size_t index = 0; index < tokenizer.count; )
    {
        uint64_t token = tokenizer.data[index];
        uint8_t tokenType = TOML_TOKEN_TYPE(token);
        string s = tokenizer.dict.data[TOML_TOKEN_INDEX(token)];
        if (tokenType == TOML_TOKEN_TYPE_DEFAULT && s[0] == '[')
        {
            char path[1024];
            index = toml_file_tokenizer_resolve_path(&tokenizer, index + 1, ']', path) + 1;
            currentSection = toml_section_add_section(&root, path);
            continue;
        }
        if (tokenType == TOML_TOKEN_TYPE_KEY)
        {
            char keyPath[1024];
            index = toml_file_tokenizer_resolve_path(&tokenizer, index, '=', keyPath) + 1;

            toml_property* p = toml_section_add_property(currentSection, keyPath);
            index = toml_property_parse_value(&tokenizer, index, p);
            continue;
        }
        index++;
    }

    toml_section_invalidate_parents(NULL, &root);

    toml_file_tokenizer_dispose(&tokenizer);
    return root;
}

#endif

#endif
