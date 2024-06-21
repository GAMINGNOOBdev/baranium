#include <argument_parser.h>
#include <stdlib.h>
#include <memory.h>

Argument* CreateArgument(ArgumentType type, const char* name, const char* secondName, const char* value)
{
    Argument* obj = (Argument*)malloc(sizeof(Argument));
    if (!obj)
        return NULL;
    memset(obj, 0, sizeof(Argument));

    memcpy(&obj->Type, &type, sizeof(ArgumentType));
    memcpy(&obj->Name, &name, sizeof(const char*));
    memcpy(&obj->SecondName, &secondName, sizeof(const char*));
    memcpy(&obj->Value, &value, sizeof(const char*));

    return obj;
}

void DisposeArgument(Argument* obj)
{
    if (!obj) return;

    free(obj);
}

/////////////////////
/// Argument List ///
/////////////////////

struct ArgumentList* argument_list_init()
{
    struct ArgumentList* obj = malloc(sizeof(struct ArgumentList));
    if (!obj) return NULL;

    memset(obj, 0, sizeof(struct ArgumentList));

    obj->size = 0;
    obj->start = NULL;
    obj->end = NULL;

    return obj;
}

void argument_list_clear(struct ArgumentList* obj)
{
    if (!obj) return;

    if (obj->start == NULL)
        return;

    Argument* next = NULL;
    for (Argument* ptr = obj->start; ptr != NULL;)
    {
        next = ptr->next;
        DisposeArgument(ptr);
        ptr = next;
    }

    obj->start = obj->end = NULL;
    obj->size = 0;
}

void argument_list_dispose(struct ArgumentList* obj)
{
    if (!obj) return;

    argument_list_clear(obj);
    free(obj);
}

void argument_list_add(struct ArgumentList* obj, Argument* arg)
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

    arg->prev = obj->end;
    arg->next = NULL;
    obj->end->next = arg;
    obj->end = arg;
    obj->size++;
}

void argument_list_remove(struct ArgumentList* obj, Argument* arg)
{
    if (obj == NULL)
        return;

    if (obj->end == NULL)
        return;

    Argument* entry = obj->start;
    Argument* next = NULL;
    for (;entry != NULL && entry != arg;)
    {
        next = entry->next;
        entry = next;
    }

    if (entry == NULL)
        return;

    Argument* prev = entry->prev;
    if (entry == obj->start && entry == obj->end)
    {
        obj->size = 0;
        obj->start = obj->end = NULL;
        DisposeArgument(entry);
        return;
    }
    else if (entry == obj->start)
    {
        next->prev = NULL;
        obj->start = next;
        entry->next = NULL;
        obj->size--;
        DisposeArgument(entry);
        return;
    }
    else if (entry == obj->end)
    {
        prev->next = NULL;
        obj->end = prev;
        entry->prev = NULL;
        obj->size--;
        DisposeArgument(entry);
        return;
    }

    prev->next = next;
    next->prev = prev;
    entry->next = entry->prev = NULL;
    obj->size--;
    DisposeArgument(entry);
}

///////////////////////
/// Argument Parser ///
///////////////////////

ArgumentParser* argument_parser_init()
{
    ArgumentParser* obj = malloc(sizeof(ArgumentParser));
    if (!obj) return NULL;

    memset(obj, 0, sizeof(ArgumentParser));

    obj->unparsed = argument_list_init();
    obj->parsed = argument_list_init();
    obj->lookup = argument_list_init();

    return obj;
}

void argument_parser_dispose(ArgumentParser* obj)
{
    if (!obj) return;

    argument_list_dispose(obj->unparsed);
    argument_list_dispose(obj->parsed);
    argument_list_dispose(obj->lookup);
    free(obj);
}

void argument_parser_add(ArgumentParser* obj, ArgumentType type, const char* name, const char* alternateName)
{
    if (!obj || !name) return;

    Argument* arg = CreateArgument(type, name, alternateName, "");
    argument_list_add(obj->lookup, arg);
}

Argument* get_matching_argument(struct ArgumentList* list, const char* name)
{
    if (!list || !name) return NULL;
    if (!list->size) return NULL;

    for (Argument* ptr = list->start; ptr != NULL; ptr = ptr->next)
    {
        if (ptr->Name == NULL)
            continue;
        
        if (strcmp(name, ptr->Name) == 0)
            return ptr;

        if (ptr->SecondName != NULL)
            if (strcmp(name, ptr->SecondName) == 0)
                return ptr;
    }

    return NULL;
}

void argument_parser_parse(ArgumentParser* obj, int argc, const char** argv)
{
    if (!obj) return;

    argument_list_clear(obj->unparsed);
    argument_list_clear(obj->parsed);

    for (int i = 1; i < argc; i++)
    {
        const char* argStr = argv[i];
        if (argStr[0] != '-')
        {
            argument_list_add(obj->unparsed, CreateArgument(ArgumentType_Invalid, argStr, argStr, argStr));
            continue;
        }

        Argument* lookupArgument = get_matching_argument(obj->lookup, argStr);
        if (!lookupArgument)
            continue;
        
        if (lookupArgument->Type == ArgumentType_Value)
        {
            if (i == argc - 1) break;

            argument_list_add(obj->parsed, CreateArgument(ArgumentType_Value, lookupArgument->Name, lookupArgument->SecondName, argv[i+1]));
            i++;
            continue;
        }

        argument_list_add(obj->parsed, CreateArgument(ArgumentType_Flag, lookupArgument->Name, lookupArgument->SecondName, ""));
    }
}

Argument* argument_parser_get(ArgumentParser* obj, const char* name)
{
    if (!obj)
        return NULL;

    return get_matching_argument(obj->parsed, name);
}

uint8_t argument_parser_has(ArgumentParser* obj, const char* name)
{
    return argument_parser_get(obj, name) != NULL;
}