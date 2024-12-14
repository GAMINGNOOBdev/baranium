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

struct ArgumentList
{
    Argument* start;
    Argument* end;
    size_t size;
};

typedef struct ArgumentParser
{
    struct ArgumentList* unparsed;
    struct ArgumentList* parsed;
    struct ArgumentList* lookup;
} ArgumentParser;

/**
 * @brief Create a new argument parser
 */
ArgumentParser* argument_parser_init();

/**
 * @brief Dispose an argument parser object
 */
void argument_parser_dispose(ArgumentParser* obj);

/**
 * @brief Add an argument to the parser
 * 
 * @param obj Parser object
 * @param type Argument type
 * @param name Argument name
 * @param alternateName Alternative argument name
 */
void argument_parser_add(ArgumentParser* obj, ArgumentType type, const char* name, const char* alternateName);

/**
 * @brief Parse the command line arguments
 * 
 * @note No need to modify the argc and argv variables
 * 
 * @param obj Parser object
 * @param argc Argument count
 * @param argv Arguments
 */
void argument_parser_parse(ArgumentParser* obj, int argc, const char** argv);

/**
 * @brief Get a parsed argument (if available)
 * 
 * @param obj Parser object
 * @param name Argument name (either normal or alternative one)
 */
Argument* argument_parser_get(ArgumentParser* obj, const char* name);

/**
 * @brief Check if an argument is parsed/existent
 * 
 * @param obj Parser object
 * @param name Argument name (either normal or alternative one)
 */
uint8_t argument_parser_has(ArgumentParser* obj, const char* name);

#endif
