#ifndef __ARGUMENT_PARSER_H_
#define __ARGUMENT_PARSER_H_ 1

#include <stdint.h>
#include <stddef.h>

typedef enum ArgumentType
{
    ArgumentType_Invalid,
    ArgumentType_Value,
    ArgumentType_Flag
} ArgumentType;

typedef struct Argument
{
    struct Argument* prev;
    ArgumentType Type;
    const char* Name;
    const char* SecondName;
    const char* Value;
    struct Argument* next;
} Argument;

typedef struct ArgumentList
{
    Argument* start;
    Argument* end;
    size_t size;
} ArgumentList;

typedef struct ArgumentParser
{
    ArgumentList unparsed;
    ArgumentList parsed;
    ArgumentList lookup;
} ArgumentParser;

void argument_parser_init(ArgumentParser* obj);
void argument_parser_dispose(ArgumentParser* obj);
void argument_parser_add(ArgumentParser* obj, ArgumentType type, const char* name, const char* alternateName);
void argument_parser_parse(ArgumentParser* obj, int argc, const char** argv);
Argument* argument_parser_get(ArgumentParser* obj, const char* name);
uint8_t argument_parser_has(ArgumentParser* obj, const char* name);

#endif