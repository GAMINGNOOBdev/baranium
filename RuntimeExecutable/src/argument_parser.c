#include <argument_parser.h>
#include <stdlib.h>
#include <memory.h>

argument* create_argument(argument_type type, const char* name, const char* secondName, const char* value)
{
    argument* obj = (argument*)malloc(sizeof(argument));
    if (!obj)
        return NULL;
    memset(obj, 0, sizeof(argument));

    memcpy(&obj->type, &type, sizeof(argument_type));
    memcpy(&obj->name, &name, sizeof(const char*));
    memcpy(&obj->second_name, &secondName, sizeof(const char*));
    memcpy(&obj->values[0], &value, sizeof(const char*));
    obj->value_count++;

    return obj;
}

void dispose_argument(argument* obj)
{
    if (!obj) return;

    free(obj);
}

/////////////////////
/// argument List ///
/////////////////////

argument* get_matching_argument(argument_list* list, const char* name);

argument_list* argument_list_init(void)
{
    argument_list* obj = malloc(sizeof(argument_list));
    if (!obj) return NULL;

    memset(obj, 0, sizeof(argument_list));

    obj->size = 0;
    obj->start = NULL;
    obj->end = NULL;

    return obj;
}

void argument_list_clear(argument_list* obj)
{
    if (!obj) return;

    if (obj->start == NULL)
        return;

    argument* next = NULL;
    for (argument* ptr = obj->start; ptr != NULL;)
    {
        next = ptr->next;
        dispose_argument(ptr);
        ptr = next;
    }

    obj->start = obj->end = NULL;
    obj->size = 0;
}

void argument_list_dispose(argument_list* obj)
{
    if (!obj) return;

    argument_list_clear(obj);
    free(obj);
}

void argument_list_add(argument_list* obj, argument* arg)
{
    if (!obj) return;
    if (!arg) return;

    if (obj->start == NULL)
    {
        arg->next = NULL;
        arg->prev = NULL;
        obj->start = arg;
        obj->end = obj->start;
        obj->size++;
        return;
    }

    argument* tmparg = get_matching_argument(obj, arg->name);
    if (tmparg != NULL)
    {
        tmparg->values[tmparg->value_count] = arg->values[0];
        tmparg->value_count++;
        return;
    }

    arg->prev = obj->end;
    arg->next = NULL;
    obj->end->next = arg;
    obj->end = arg;
    obj->size++;
}

void argument_list_remove(argument_list* obj, argument* arg)
{
    if (obj == NULL)
        return;

    if (obj->end == NULL)
        return;

    argument* entry = obj->start;
    argument* next = NULL;
    for (;entry != NULL && entry != arg;)
    {
        next = entry->next;
        entry = next;
    }

    if (entry == NULL)
        return;

    argument* prev = entry->prev;
    if (entry == obj->start && entry == obj->end)
    {
        obj->size = 0;
        obj->start = obj->end = NULL;
        dispose_argument(entry);
        return;
    }
    else if (entry == obj->start)
    {
        next->prev = NULL;
        obj->start = next;
        entry->next = NULL;
        obj->size--;
        dispose_argument(entry);
        return;
    }
    else if (entry == obj->end)
    {
        prev->next = NULL;
        obj->end = prev;
        entry->prev = NULL;
        obj->size--;
        dispose_argument(entry);
        return;
    }

    prev->next = next;
    next->prev = prev;
    entry->next = entry->prev = NULL;
    obj->size--;
    dispose_argument(entry);
}

///////////////////////
/// argument Parser ///
///////////////////////

argument_parser* argument_parser_init(void)
{
    argument_parser* obj = malloc(sizeof(argument_parser));
    if (!obj) return NULL;

    memset(obj, 0, sizeof(argument_parser));

    obj->unparsed = argument_list_init();
    obj->parsed = argument_list_init();
    obj->lookup = argument_list_init();

    return obj;
}

void argument_parser_dispose(argument_parser* obj)
{
    if (!obj) return;

    argument_list_dispose(obj->unparsed);
    argument_list_dispose(obj->parsed);
    argument_list_dispose(obj->lookup);
    free(obj);
}

void argument_parser_add(argument_parser* obj, argument_type type, const char* name, const char* alternateName)
{
    if (!obj || !name) return;

    argument* arg = create_argument(type, name, alternateName, "");
    argument_list_add(obj->lookup, arg);
}

argument* get_matching_argument(argument_list* list, const char* name)
{
    if (!list || !name) return NULL;
    if (!list->size) return NULL;

    for (argument* ptr = list->start; ptr != NULL; ptr = ptr->next)
    {
        if (ptr->name == NULL)
            continue;
        
        if (strcmp(name, ptr->name) == 0)
            return ptr;

        if (ptr->second_name != NULL)
            if (strcmp(name, ptr->second_name) == 0)
                return ptr;
    }

    return NULL;
}

void argument_parser_parse(argument_parser* obj, int argc, const char** argv)
{
    if (!obj) return;

    argument_list_clear(obj->unparsed);
    argument_list_clear(obj->parsed);

    for (int i = 1; i < argc; i++)
    {
        const char* argStr = argv[i];
        if (argStr[0] != '-')
        {
            argument_list_add(obj->unparsed, create_argument(Argument_Type_Invalid, argStr, argStr, argStr));
            continue;
        }

        argument* lookupargument = get_matching_argument(obj->lookup, argStr);
        if (!lookupargument)
            continue;

        if (lookupargument->type == Argument_Type_Value)
        {
            if (i == argc - 1) break;

            argument_list_add(obj->parsed, create_argument(Argument_Type_Value, lookupargument->name, lookupargument->second_name, argv[i+1]));
            i++;
            continue;
        }

        argument_list_add(obj->parsed, create_argument(Argument_Type_Flag, lookupargument->name, lookupargument->second_name, ""));
    }
}

argument* argument_parser_get(argument_parser* obj, const char* name)
{
    if (!obj)
        return NULL;

    return get_matching_argument(obj->parsed, name);
}

uint8_t argument_parser_has(argument_parser* obj, const char* name)
{
    return argument_parser_get(obj, name) != NULL;
}
