#include <config.h>
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

int config_estimate_value_type(const char* value)
{
    size_t valueLen = strlen(value);
    if (value == NULL || valueLen == 0)
        return CONFIG_PROPERTY_VALUE_TYPE_UNKNOWN;

    if (is_bool(value))
        return CONFIG_PROPERTY_VALUE_TYPE_BOOL;

    if (is_integer(value))
        return CONFIG_PROPERTY_VALUE_TYPE_INT;

    if (is_float(value))
        return CONFIG_PROPERTY_VALUE_TYPE_FLOAT;

    if (value[0] == '[' && value[valueLen-1] == ']')
        return CONFIG_PROPERTY_VALUE_TYPE_ARRAY;

    if ((value[0] == '\'' || value[0] == '"') && (value[valueLen-1] == '\'' || value[valueLen-1] == '"'))
        return CONFIG_PROPERTY_VALUE_TYPE_STRING;

    return CONFIG_PROPERTY_VALUE_TYPE_STRING;
}

const char* config_property_value_type_as_string(int type)
{
    if (type == CONFIG_PROPERTY_VALUE_TYPE_STRING)
        return "STRING";

    if (type == CONFIG_PROPERTY_VALUE_TYPE_FLOAT)
        return "FLOAT";

    if (type == CONFIG_PROPERTY_VALUE_TYPE_BOOL)
        return "BOOL";

    if (type == CONFIG_PROPERTY_VALUE_TYPE_INT)
        return "INT";

    if (type == CONFIG_PROPERTY_VALUE_TYPE_ARRAY)
        return "ARRAY";

    return "UNKNOWN";
}

///////////////////////////
///                     ///
///   Config Property   ///
///                     ///
///////////////////////////

void config_property_dispose(config_property_t* property)
{
    if (property == NULL)
        return;

    iffree((void*)property->name);

    for (int i = 0; i < property->arrayLength; i++)
        config_property_dispose(&property->value.arrayValue[i]);

    if (property->type == CONFIG_PROPERTY_VALUE_TYPE_STRING || property->type == CONFIG_PROPERTY_VALUE_TYPE_UNKNOWN)
        iffree((void*)property->value.stringValue);

    if (property->type == CONFIG_PROPERTY_VALUE_TYPE_ARRAY)
        iffree((void*)property->value.arrayValue);

    memset(property, 0, sizeof(config_property_t));
}

config_property_t config_property_duplicate(config_property_t* property)
{
    config_property_t result = CONFIG_PROPERTY_EMPTY;
    if (property == NULL)
        return result;

    if (property->name)
        result.name = strdup(property->name);
    result.arrayLength = property->arrayLength;
    result.type = property->type;
    result.parent = property->parent;
    result.value = property->value;

    if (result.type == CONFIG_PROPERTY_VALUE_TYPE_ARRAY)
    {
        result.value.arrayValue = (config_property_t*)malloc(sizeof(config_property_t)*result.arrayLength);
        for (int i = 0; i < result.arrayLength; i++)
            result.value.arrayValue[i] = config_property_duplicate(&property->value.arrayValue[i]);
        return result;
    }

    if (result.type == CONFIG_PROPERTY_VALUE_TYPE_UNKNOWN || result.type == CONFIG_PROPERTY_VALUE_TYPE_STRING)
    {
        result.value.stringValue = strdup(result.value.stringValue);
        return result;
    }

    return result;
}

const char* config_property_get_full_name(config_property_t* property)
{
    if (property == NULL)
        return NULL;

    if (property->parent == NULL)
        return strdup(property->name);

    char* parentSectionName = (char*)config_section_get_full_name(property->parent);
    char* parent = strdup(parentSectionName);
    parent = (char*)realloc(parent, strlen(parent) + strlen(property->name) + 2);
    free(parentSectionName);
    strcat(parent, ".");
    strcat(parent, property->name);
    return parent;
}

void config_property_array_add(config_property_t* property, config_property_t* other)
{
    if (property == NULL || other == NULL)
        return;

    if (property->type != CONFIG_PROPERTY_VALUE_TYPE_ARRAY)
        return;

    property->arrayLength++;
    property->value.arrayValue = (config_property_t*)realloc(property->value.arrayValue, sizeof(config_property_t)*property->arrayLength);
    property->value.arrayValue[property->arrayLength-1] = config_property_duplicate(other);
}

void config_property_array_insert_at(config_property_t* property, int index, config_property_t* other)
{
    if (property == NULL || index < 0 || other == NULL)
        return;

    if (property->type != CONFIG_PROPERTY_VALUE_TYPE_ARRAY)
        return;

    if ((property->arrayLength == 0 && index == 0) || index == -1)
    {
        config_property_array_add(property, other);
        return;
    }

    if (index >= property->arrayLength)
        return;

    property->arrayLength++;
    property->value.arrayValue = (config_property_t*)realloc(property->value.arrayValue, sizeof(config_property_t)*property->arrayLength);
    memmove(&property->value.arrayValue[index+1], &property->value.arrayValue[index], sizeof(config_property_t)*(property->arrayLength-index-1));
    property->value.arrayValue[index] = config_property_duplicate(other);
}

void config_property_array_remove(config_property_t* property, int index)
{
    if (property == NULL)
        return;

    if (property->type != CONFIG_PROPERTY_VALUE_TYPE_ARRAY)
        return;

    if (property->arrayLength == 0)
        return;

    if (index == -1 || index == property->arrayLength-1)
    {
        config_property_dispose(&property->value.arrayValue[property->arrayLength-1]);
        property->arrayLength--;
        property->value.arrayValue = (config_property_t*)realloc(property->value.arrayValue, sizeof(config_property_t)*property->arrayLength);
        return;
    }
    else if (index == 0)
    {
        config_property_t tmp = property->value.arrayValue[0];
        config_property_dispose(&tmp);
        memmove(property->value.arrayValue, &property->value.arrayValue[1], sizeof(config_property_t)*property->arrayLength-1);
        property->arrayLength--;
        property->value.arrayValue = (config_property_t*)realloc(property->value.arrayValue, sizeof(config_property_t)*property->arrayLength);
        return;
    }
    else if (index >= property->arrayLength)
        return;

    config_property_t tmp = property->value.arrayValue[index];
    config_property_dispose(&tmp);
    memmove(&property->value.arrayValue[index], &property->value.arrayValue[index+1], sizeof(config_property_t)*(property->arrayLength-index-1));
    property->arrayLength--;
    property->value.arrayValue = (config_property_t*)realloc(property->value.arrayValue, sizeof(config_property_t)*property->arrayLength);
}

void config_property_set_value_from_string(config_property_t* property, const char* string)
{
    if (property == NULL || string == NULL)
        return;

    char* value = strdup(string);
    char* valueog = value; // in case we modify it if the `value` pointer
    strtrim(value);
    property->type = config_estimate_value_type(value);
    char* tmp;
    switch (property->type)
    {
    case CONFIG_PROPERTY_VALUE_TYPE_INT:
    {
        property->value.intValue = strtol(value, &tmp, 10);
        break;
    }

    case CONFIG_PROPERTY_VALUE_TYPE_FLOAT:
    {
        property->value.floatValue = strtof(value, &tmp);
        break;
    }

    case CONFIG_PROPERTY_VALUE_TYPE_BOOL:
    {
        property->value.boolValue = 0;
        if (strcmp(value, "true") == 0 || strcmp(value, "True") == 0)
            property->value.boolValue = 1;
        break;
    }

    case CONFIG_PROPERTY_VALUE_TYPE_ARRAY:
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
            property->value.arrayValue = (config_property_t*)realloc(property->value.arrayValue, sizeof(config_property_t)*property->arrayLength);
            config_property_t tmp = CONFIG_PROPERTY_EMPTY;
            config_property_set_value_from_string(&tmp, value);
            memcpy(&property->value.arrayValue[property->arrayLength-1], &tmp, sizeof(config_property_t));
        }
        else
        {
            value2[0] = 0;
            value2++;
            property->arrayLength++;
            property->value.arrayValue = (config_property_t*)realloc(property->value.arrayValue, sizeof(config_property_t)*property->arrayLength);
            config_property_t tmp = CONFIG_PROPERTY_EMPTY;
            config_property_set_value_from_string(&tmp, value);
            memcpy(&property->value.arrayValue[property->arrayLength-1], &tmp, sizeof(config_property_t));
            for (;;)
            {
                char* value3 = strchr(value2, ',');
                if (value3 == NULL)
                    break;
                value3[0] = 0;
                value3++;
                property->arrayLength++;
                property->value.arrayValue = (config_property_t*)realloc(property->value.arrayValue, sizeof(config_property_t)*property->arrayLength);
                tmp = CONFIG_PROPERTY_EMPTY;
                config_property_set_value_from_string(&tmp, value2);
                memcpy(&property->value.arrayValue[property->arrayLength-1], &tmp, sizeof(config_property_t));
                value2 = value3;
            }

            property->arrayLength++;
            property->value.arrayValue = (config_property_t*)realloc(property->value.arrayValue, sizeof(config_property_t)*property->arrayLength);
            tmp = CONFIG_PROPERTY_EMPTY;
            config_property_set_value_from_string(&tmp, value2);
            memcpy(&property->value.arrayValue[property->arrayLength-1], &tmp, sizeof(config_property_t));
        }
        break;
    }

    case CONFIG_PROPERTY_VALUE_TYPE_UNKNOWN:
    case CONFIG_PROPERTY_VALUE_TYPE_STRING:
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

void config_property_set_string(config_property_t* property, const char* value)
{
    if (property == NULL)
        return;

    if (property->type == CONFIG_PROPERTY_VALUE_TYPE_STRING || property->type == CONFIG_PROPERTY_VALUE_TYPE_UNKNOWN)
        iffree((void*)property->value.stringValue);

    if (property->type == CONFIG_PROPERTY_VALUE_TYPE_ARRAY)
        iffree((void*)property->value.arrayValue);

    property->arrayLength = 0;
    property->type = CONFIG_PROPERTY_VALUE_TYPE_STRING;
    if (value == NULL)
    {
        property->value.stringValue = NULL;
        return;
    }

    property->value.stringValue = strdup(value);
}

void config_property_set_float(config_property_t* property, float value)
{
    if (property == NULL)
        return;

    if (property->type == CONFIG_PROPERTY_VALUE_TYPE_STRING || property->type == CONFIG_PROPERTY_VALUE_TYPE_UNKNOWN)
        iffree((void*)property->value.stringValue);

    if (property->type == CONFIG_PROPERTY_VALUE_TYPE_ARRAY)
        iffree((void*)property->value.arrayValue);

    property->arrayLength = 0;
    property->type = CONFIG_PROPERTY_VALUE_TYPE_FLOAT;
    property->value.floatValue = value;
}

void config_property_set_bool(config_property_t* property, uint8_t value)
{
    if (property == NULL)
        return;

    if (property->type == CONFIG_PROPERTY_VALUE_TYPE_STRING || property->type == CONFIG_PROPERTY_VALUE_TYPE_UNKNOWN)
        iffree((void*)property->value.stringValue);

    if (property->type == CONFIG_PROPERTY_VALUE_TYPE_ARRAY)
        iffree((void*)property->value.arrayValue);

    property->arrayLength = 0;
    property->type = CONFIG_PROPERTY_VALUE_TYPE_BOOL;
    property->value.boolValue = value;
}

void config_property_set_int(config_property_t* property, int value)
{
    if (property == NULL)
        return;

    if (property->type == CONFIG_PROPERTY_VALUE_TYPE_STRING || property->type == CONFIG_PROPERTY_VALUE_TYPE_UNKNOWN)
        iffree((void*)property->value.stringValue);

    if (property->type == CONFIG_PROPERTY_VALUE_TYPE_ARRAY)
        iffree((void*)property->value.arrayValue);

    property->arrayLength = 0;
    property->type = CONFIG_PROPERTY_VALUE_TYPE_INT;
    property->value.intValue = value;
}

void config_property_set_array(config_property_t* property, int len, config_property_t array[])
{
    if (property == NULL)
        return;

    if (property->type == CONFIG_PROPERTY_VALUE_TYPE_STRING || property->type == CONFIG_PROPERTY_VALUE_TYPE_UNKNOWN)
        iffree((void*)property->value.stringValue);

    if (property->type == CONFIG_PROPERTY_VALUE_TYPE_ARRAY)
        iffree((void*)property->value.arrayValue);

    property->arrayLength = len;
    property->type = CONFIG_PROPERTY_VALUE_TYPE_ARRAY;
    property->value.arrayValue = (config_property_t*)malloc(sizeof(config_property_t)*len);
    for (int i = 0; i < len; i++)
        property->value.arrayValue[i] = config_property_duplicate(&array[i]);
}

void config_property_save_value(config_property_t* property, FILE* file)
{
    if (property == NULL || file == NULL)
        return;

    switch (property->type)
    {
    case CONFIG_PROPERTY_VALUE_TYPE_INT:
    {
        fprintf(file, "%d", property->value.intValue);
        break;
    }

    case CONFIG_PROPERTY_VALUE_TYPE_FLOAT:
    {
        fprintf(file, "%f", property->value.floatValue);
        break;
    }

    case CONFIG_PROPERTY_VALUE_TYPE_BOOL:
    {
        fprintf(file, "%s", property->value.boolValue ? "true" : "false");
        break;
    }

    case CONFIG_PROPERTY_VALUE_TYPE_ARRAY:
    {
        fprintf(file, "[ ");
        for (int i = 0; i < property->arrayLength; i++)
        {
            config_property_t* tmp = &property->value.arrayValue[i];
            config_property_save_value(tmp, file);
            if (i != property->arrayLength - 1)
                fprintf(file, ", ");
        }
        fprintf(file, " ]");
        break;
    }

    case CONFIG_PROPERTY_VALUE_TYPE_UNKNOWN:
    case CONFIG_PROPERTY_VALUE_TYPE_STRING:
    {
        if (property->value.stringValue == NULL || strlen(property->value.stringValue) < 1)
            fprintf(file, "''");
        else
            fprintf(file, "'%s'", property->value.stringValue);
        break;
    }
    }
}

void config_property_save(config_property_t* property, FILE* file)
{
    if (property == NULL || file == NULL)
        return;

    fprintf(file, "%s = ", property->name);
    config_property_save_value(property, file);
    fprintf(file, "\n");
    fflush(file);
}

int config_property_cmp(config_property_t* a, config_property_t* b)
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

void config_section_dispose(config_section_t* section)
{
    if (section == NULL)
        return;

    for (size_t i = 0; i < section->sectionCount; i++)
        config_section_dispose(&section->sections[i]);

    for (size_t i = 0; i < section->propertyCount; i++)
        config_property_dispose(&section->properties[i]);

    iffree((void*)section->name);
    iffree((void*)section->properties);
    iffree((void*)section->sections);

    memset(section, 0, sizeof(config_section_t));
}

const char* config_section_get_full_name(config_section_t* section)
{
    if (section == NULL)
        return NULL;

    if (section->parent == NULL)
        return strdup(section->name);

    char* parentSectionName = (char*)config_section_get_full_name(section->parent);
    char* parent = strdup(parentSectionName);
    parent = (char*)realloc(parent, strlen(parent) + strlen(section->name) + 2);
    free(parentSectionName);
    parent = strcat(parent, ".");
    parent = strcat(parent, section->name);
    return parent;
}

void config_section_save(config_section_t* section, FILE* file, const char* prefix)
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
            config_property_save(&section->properties[i], file);
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
        config_section_save(&section->sections[i], file, newSectionPrefix);
    free(newSectionPrefix);

    fflush(file);
}

config_property_t* config_section_get(config_section_t* section, const char* name)
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
    if (!config_section_has_section(section, str))
    {
        free(str);
        return 0;
    }

    config_section_t* subSection = config_section_get_section(section, str);
    config_property_t* result = config_section_get(subSection, sectionDivider);
    free(str);
    return result;
}

config_section_t* config_section_get_section(config_section_t* section, const char* name)
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
    if (!config_section_has_section(section, str))
    {
        free(str);
        return NULL;
    }

    config_section_t* subSection = config_section_get_section(section, str);
    config_section_t* result = config_section_get_section(subSection, sectionDivider);
    free(str);
    return result;
}

int config_section_has_property(config_section_t* section, const char* name)
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
    if (!config_section_has_section(section, name))
    {
        free(str);
        return 0;
    }

    config_section_t* subSection = config_section_get_section(section, name);
    int result = config_section_has_property(subSection, sectionDivider);
    free(str);
    return result;
}

int config_section_has_section(config_section_t* section, const char* name)
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
    if (!config_section_has_section(section, str))
    {
        free(str);
        return 0;
    }

    config_section_t* subSection = config_section_get_section(section, str);
    int result = config_section_has_section(subSection, sectionDivider);
    free(str);
    return result;
}

config_property_t* config_section_add_property(config_section_t* section, const char* name)
{
    if (section == NULL || name == NULL || strlen(name) == 0)
        return NULL;

    if (config_section_has_section(section, name))
        return NULL;

    if (config_section_has_property(section, name))
        return config_section_get(section, name);

    config_property_t propertry = CONFIG_PROPERTY_EMPTY;
    config_property_t* propertryPtr = NULL;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        propertry.name = strdup(name);
        propertry.parent = section;

        section->propertyCount++;
        section->properties = (config_property_t*)realloc(section->properties, section->propertyCount*sizeof(config_property_t));
        propertryPtr = &section->properties[section->propertyCount-1];
        memcpy(propertryPtr, &propertry, sizeof(config_property_t));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        config_section_t* nextSection = NULL;
        if (!config_section_has_section(section, str))
            nextSection = config_section_add_section(section, str);
        else
            nextSection = config_section_get_section(section, str);

        if (nextSection == NULL)
        {
            free(str);
            return NULL;
        }

        propertryPtr = config_section_add_property(nextSection, sectionDivider);
    }

    free(str);
    return propertryPtr;
}

config_section_t* config_section_add_section(config_section_t* section, const char* name)
{
    if (section == NULL || name == NULL || strlen(name) == 0)
        return NULL;

    if (config_section_has_section(section, name))
        return config_section_get_section(section, name);

    config_section_t newSection = CONFIG_SECTION_EMPTY;
    config_section_t* newSectionPtr = NULL;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        newSection.name = strdup(name);
        newSection.parent = section;

        section->sectionCount++;
        section->sections = (config_section_t*)realloc(section->sections, section->sectionCount*sizeof(config_section_t));
        newSectionPtr = &section->sections[section->sectionCount-1];
        memcpy(newSectionPtr, &newSection, sizeof(config_section_t));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        config_section_t* nextSection = NULL;
        if (!config_section_has_section(section, str))
            nextSection = config_section_add_section(section, str);
        else
            nextSection = config_section_get_section(section, str);

        if (nextSection == NULL)
        {
            free(str);
            return NULL;
        }

        newSectionPtr = config_section_add_section(nextSection, sectionDivider);
    }

    free(str);
    return newSectionPtr;
}

void config_section_remove_property(config_section_t* section, const char* name)
{
    if (section == NULL || name == NULL || strlen(name) == 0)
        return;

    if (config_section_has_section(section, name))
        return;

    if (!config_section_has_property(section, name))
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
        config_property_dispose(&section->properties[index]);
        if (index != section->propertyCount-1)
        {
            config_property_t tmp = section->properties[section->propertyCount-1];
            section->properties[section->propertyCount-1] = section->properties[index];
            section->properties[index] = tmp;
        }

        section->propertyCount--;
        section->properties = (config_property_t*)realloc(section->properties, section->propertyCount*sizeof(config_property_t));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        config_section_t* nextSection = config_section_get_section(section, str);
        if (nextSection == NULL)
        {
            free(str);
            return;
        }

        config_section_remove_property(nextSection, sectionDivider);
    }

    free(str);
}

void config_section_remove_section(config_section_t* section, const char* name)
{
    if (section == NULL || name == NULL || strlen(name) == 0)
        return;

    if (!config_section_has_section(section, name))
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
        config_section_dispose(&section->sections[index]);
        if (index != section->sectionCount-1)
        {
            config_section_t tmp = section->sections[section->sectionCount-1];
            section->sections[section->sectionCount-1] = section->sections[index];
            section->sections[index] = tmp;
        }

        section->sectionCount--;
        section->sections = (config_section_t*)realloc(section->sections, section->sectionCount*sizeof(config_section_t));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        config_section_t* nextSection = config_section_get_section(section, str);
        if (nextSection == NULL)
        {
            free(str);
            return;
        }

        config_section_remove_section(nextSection, sectionDivider);
    }

    free(str);
}

int config_section_cmp(config_section_t* a, config_section_t* b)
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

void config_file_close(config_file_t* config)
{
    if (config == NULL)
        return;

    for (size_t i = 0; i < config->sectionCount; i++)
        config_section_dispose(&config->sections[i]);

    for (size_t i = 0; i < config->propertyCount; i++)
        config_property_dispose(&config->properties[i]);

    iffree(config->sections);
    iffree(config->properties);

    memset(config, 0, sizeof(config_file_t));
}

void config_file_open(config_file_t* config, FILE* file)
{
    if (config == NULL || file == NULL)
        return;

    config_section_t* currentSection = NULL;
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

            if (config_file_has_section(config, line))
                currentSection = config_file_get_section(config, line);
            else
                currentSection = config_file_add_section(config, line);

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

        config_property_t* property = NULL;
        if (currentSection != NULL)
            property = config_section_add_property(currentSection, split0);
        else
            property = config_file_add_property(config, split0);

        config_property_set_value_from_string(property, split1);
    }
}

void config_file_save(config_file_t* config, FILE* file)
{
    if (config == NULL || file == NULL)
        return;

    for (size_t i = 0; i < config->propertyCount; i++)
        config_property_save(&config->properties[i], file);

    if (config->propertyCount != 0)
        fprintf(file, "\n");

    fflush(file);

    for (size_t i = 0; i < config->sectionCount; i++)
        config_section_save(&config->sections[i], file, NULL);

    fflush(file);
}

config_property_t* config_file_get(config_file_t* config, const char* name)
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
    if (!config_file_has_section(config, str))
    {
        free(str);
        return NULL;
    }

    config_section_t* subSection = config_file_get_section(config, str);
    config_property_t* result = config_section_get(subSection, sectionDivider);
    free(str);
    return result;
}

config_section_t* config_file_get_section(config_file_t* config, const char* name)
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
    if (!config_file_has_section(config, str))
    {
        free(str);
        return NULL;
    }

    config_section_t* subSection = config_file_get_section(config, str);
    config_section_t* result = config_section_get_section(subSection, sectionDivider);
    free(str);
    return result;
}

int config_file_has_property(config_file_t* config, const char* name)
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
    if (!config_file_has_section(config, str))
    {
        free(str);
        return 0;
    }

    config_section_t* subSection = config_file_get_section(config, str);
    int result = config_section_has_property(subSection, sectionDivider);
    free(str);
    return result;
}

int config_file_has_section(config_file_t* config, const char* name)
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
    if (!config_file_has_section(config, str))
    {
        free(str);
        return 0;
    }

    config_section_t* subSection = config_file_get_section(config, str);
    int result = config_section_has_section(subSection, sectionDivider);
    free(str);
    return result;
}

config_property_t* config_file_add_property(config_file_t* config, const char* name)
{
    if (config == NULL || name == NULL || strlen(name) == 0)
        return NULL;

    if (config_file_has_section(config, name))
        return NULL;

    if (config_file_has_property(config, name))
        return config_file_get(config, name);

    config_property_t propertry = CONFIG_PROPERTY_EMPTY;
    config_property_t* propertryPtr = NULL;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        propertry.name = strdup(name);

        config->propertyCount++;
        config->properties = (config_property_t*)realloc(config->properties, config->propertyCount*sizeof(config_property_t));
        propertryPtr = &config->properties[config->propertyCount-1];
        memcpy(propertryPtr, &propertry, sizeof(config_property_t));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        config_section_t* nextSection = NULL;
        if (!config_file_has_section(config, str))
            nextSection = config_file_add_section(config, str);
        else
            nextSection = config_file_get_section(config, str);

        if (nextSection == NULL)
        {
            free(str);
            return NULL;
        }

        propertryPtr = config_section_add_property(nextSection, sectionDivider);
    }

    free(str);
    return propertryPtr;
}

config_section_t* config_file_add_section(config_file_t* config, const char* name)
{
    if (config == NULL || name == NULL || strlen(name) == 0)
        return NULL;

    if (config_file_has_section(config, name))
        return config_file_get_section(config, name);

    config_section_t newSection = CONFIG_SECTION_EMPTY;
    config_section_t* newSectionPtr = NULL;

    char* str = strdup(name);
    char* sectionDivider = (char*)strchr(str, '.');
    if (sectionDivider == NULL)
    {
        newSection.name = strdup(name);

        config->sectionCount++;
        config->sections = (config_section_t*)realloc(config->sections, config->sectionCount*sizeof(config_section_t));
        newSectionPtr = &config->sections[config->sectionCount-1];
        memcpy(newSectionPtr, &newSection, sizeof(config_section_t));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        config_section_t* nextSection = NULL;
        if (!config_file_has_section(config, str))
            nextSection = config_file_add_section(config, str);
        else
            nextSection = config_file_get_section(config, str);

        if (nextSection == NULL)
        {
            free(str);
            return NULL;
        }

        newSectionPtr = config_section_add_section(nextSection, sectionDivider);
    }

    free(str);
    return newSectionPtr;
}

void config_file_remove_property(config_file_t* config, const char* name)
{
    if (config == NULL || name == NULL || strlen(name) == 0)
        return;

    if (config_file_has_section(config, name))
        return;

    if (!config_file_has_property(config, name))
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
        config_property_dispose(&config->properties[index]);
        if (index != config->propertyCount-1)
        {
            config_property_t tmp = config->properties[config->propertyCount-1];
            config->properties[config->propertyCount-1] = config->properties[index];
            config->properties[index] = tmp;
        }

        config->propertyCount--;
        config->properties = (config_property_t*)realloc(config->properties, config->propertyCount*sizeof(config_property_t));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        config_section_t* nextSection = config_file_get_section(config, str);
        if (nextSection == NULL)
        {
            free(str);
            return;
        }

        config_section_remove_property(nextSection, sectionDivider);
    }

    free(str);
}

void config_file_remove_section(config_file_t* config, const char* name)
{
    if (config == NULL || name == NULL || strlen(name) == 0)
        return;

    if (!config_file_has_section(config, name))
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
        config_section_dispose(&config->sections[index]);
        if (index != config->sectionCount-1)
        {
            config_section_t tmp = config->sections[config->sectionCount-1];
            config->sections[config->sectionCount-1] = config->sections[index];
            config->sections[index] = tmp;
        }

        config->sectionCount--;
        config->sections = (config_section_t*)realloc(config->sections, config->sectionCount*sizeof(config_section_t));
    }
    else
    {
        sectionDivider[0] = 0;
        sectionDivider++;
        config_section_t* nextSection = config_file_get_section(config, str);
        if (nextSection == NULL)
        {
            free(str);
            return;
        }

        config_section_remove_section(nextSection, sectionDivider);
    }

    free(str);
}
