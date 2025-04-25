#ifndef __ARGUMENT_PARSER_H_
#define __ARGUMENT_PARSER_H_ 1

#include <stdint.h>
#include <stddef.h>

typedef enum
{
    Argument_Type_Invalid,
    Argument_Type_Value,
    Argument_Type_Flag
} argument_type;

typedef struct argument
{
    struct argument* prev;
    argument_type type;
    const char* name;
    const char* second_name;
    const char* values[20];
    uint8_t value_count;
    struct argument* next;
} argument;

typedef struct
{
    argument* start;
    argument* end;
    size_t size;
} argument_list;

typedef struct
{
    argument_list* unparsed;
    argument_list* parsed;
    argument_list* lookup;
} argument_parser;

/**
 * @brief Create a new argument parser
 */
argument_parser* argument_parser_init(void);

/**
 * @brief Dispose an argument parser object
 */
void argument_parser_dispose(argument_parser* obj);

/**
 * @brief Add an argument to the parser
 * 
 * @param obj Parser object
 * @param type Argument type
 * @param name Argument name
 * @param alternateName Alternative argument name
 */
void argument_parser_add(argument_parser* obj, argument_type type, const char* name, const char* alternateName);

/**
 * @brief Parse the command line arguments
 * 
 * @note No need to modify the argc and argv variables
 * 
 * @param obj Parser object
 * @param argc Argument count
 * @param argv Arguments
 */
void argument_parser_parse(argument_parser* obj, int argc, const char** argv);

/**
 * @brief Get a parsed argument (if available)
 * 
 * @param obj Parser object
 * @param name Argument name (either normal or alternative one)
 */
argument* argument_parser_get(argument_parser* obj, const char* name);

/**
 * @brief Check if an argument is parsed/existent
 * 
 * @param obj Parser object
 * @param name Argument name (either normal or alternative one)
 */
uint8_t argument_parser_has(argument_parser* obj, const char* name);

#endif
