#include <argument_parser.h>
#include <stdlib.h>
#include <memory.h>

argument_t create_argument(argument_type_t type, const char* name, const char* secondName, const char* value)
{
    argument_t arg;
    memset(&arg, 0, sizeof(argument_t));

    arg.type = type;
    arg.name = name;
    arg.second_name = secondName;
    if (value != NULL)
    {
        arg.values = malloc(sizeof(const char*));
        arg.values[0] = value;
        arg.value_count++;
    }

    return arg;
}

/////////////////////
/// argument List ///
/////////////////////

argument_t* get_matching_argument(argument_list_t* list, const char* name);

void argument_list_init(argument_list_t* obj)
{
    if (!obj)
        return;

    memset(obj, 0, sizeof(argument_list_t));
}

void argument_list_clear(argument_list_t* obj)
{
    if (!obj)
        return;

    if (obj->data == NULL)
        return;

    for (size_t i = 0; i < obj->size; i++)
        if (obj->data[i].values)
            free(obj->data[i].values);

    free(obj->data);
    argument_list_init(obj);
}

void argument_list_dispose(argument_list_t* obj)
{
    argument_list_clear(obj);
}

void argument_list_add(argument_list_t* obj, argument_t arg)
{
    if (!obj)
        return;

    argument_t* tmparg = get_matching_argument(obj, arg.name);
    if (tmparg != NULL)
    {
        tmparg->values = realloc(tmparg->values, sizeof(const char*)*tmparg->value_count+1);
        tmparg->values[tmparg->value_count] = arg.values[0];
        tmparg->value_count++;

        if (arg.values)
            free(arg.values);
        return;
    }

    if (obj->size + 1 >= obj->buffer_size)
    {
        obj->buffer_size += ARGUMENT_LIST_BUFFER_SIZE;
        obj->data = realloc(obj->data, obj->buffer_size * sizeof(argument_t));
    }

    obj->data[obj->size] = arg;
    obj->size++;
}

///////////////////////
/// argument Parser ///
///////////////////////

void argument_parser_init(argument_parser_t* obj)
{
    if (!obj)
        return;

    memset(obj, 0, sizeof(argument_parser_t));

    argument_list_init(&obj->unparsed);
    argument_list_init(&obj->parsed);
    argument_list_init(&obj->lookup);
}

void argument_parser_dispose(argument_parser_t* obj)
{
    if (!obj) return;

    argument_list_dispose(&obj->unparsed);
    argument_list_dispose(&obj->parsed);
    argument_list_dispose(&obj->lookup);

    memset(obj, 0, sizeof(argument_parser_t));
}

void argument_parser_add(argument_parser_t* obj, argument_type_t type, const char* name, const char* alternateName)
{
    if (!obj || !name)
        return;

    argument_list_add(&obj->lookup, create_argument(type, name, alternateName, NULL));
}

argument_t* get_matching_argument(argument_list_t* list, const char* name)
{
    if (!list || !list->size || !name)
        return NULL;

    for (size_t i = 0; i < list->size; i++)
    {
        argument_t arg = list->data[i];
        if (arg.name == NULL)
            continue;
        
        if (strcmp(name, arg.name) == 0)
            return &list->data[i];

        if (arg.second_name == NULL)
            continue;

        if (strcmp(name, arg.second_name) == 0)
            return &list->data[i];
    }

    return NULL;
}

void argument_parser_parse(argument_parser_t* obj, int argc, const char** argv)
{
    if (!obj) return;

    argument_list_clear(&obj->unparsed);
    argument_list_clear(&obj->parsed);

    for (int i = 1; i < argc; i++)
    {
        const char* argStr = argv[i];
        if (argStr[0] != '-')
        {
            argument_list_add(&obj->unparsed, create_argument(ARGUMENT_TYPE_INVALID, argStr, argStr, argStr));
            continue;
        }

        argument_t* lookupargument = get_matching_argument(&obj->lookup, argStr);
        if (!lookupargument)
            continue;

        if (lookupargument->type == ARGUMENT_TYPE_VALUE)
        {
            if (i == argc - 1) break;

            argument_list_add(&obj->parsed, create_argument(ARGUMENT_TYPE_VALUE, lookupargument->name, lookupargument->second_name, argv[i+1]));
            i++;
            continue;
        }

        argument_list_add(&obj->parsed, create_argument(ARGUMENT_TYPE_FLAG, lookupargument->name, lookupargument->second_name, NULL));
    }
}

argument_t* argument_parser_get(argument_parser_t* obj, const char* name)
{
    if (!obj)
        return NULL;

    return get_matching_argument(&obj->parsed, name);
}

uint8_t argument_parser_has(argument_parser_t* obj, const char* name)
{
    return argument_parser_get(obj, name) != NULL;
}
