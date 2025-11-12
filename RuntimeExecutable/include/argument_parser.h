#ifndef __ARGUMENT_PARSER_H_
#define __ARGUMENT_PARSER_H_ 1

#include <stdint.h>
#include <stddef.h>

#define ARGUMENT_LIST_BUFFER_SIZE 0xA

#define ARGUMENT_TYPE_INVALID   0
#define ARGUMENT_TYPE_VALUE     1
#define ARGUMENT_TYPE_FLAG      2

typedef int argument_type_t;

typedef struct
{
    argument_type_t type;
    const char* name;
    const char* second_name;
    uint8_t value_count;
    const char** values;
} argument_t;

typedef struct
{
    argument_t* data;
    size_t buffer_size;
    size_t size;
} argument_list_t;

typedef struct
{
    argument_list_t unparsed;
    argument_list_t parsed;
    argument_list_t lookup;
} argument_parser_t;

/**
 * @brief Create a new argument parser
 */
void argument_parser_init(argument_parser_t* obj);

/**
 * @brief Dispose an argument parser object
 */
void argument_parser_dispose(argument_parser_t* obj);

/**
 * @brief Add an argument to the parser
 * 
 * @param obj Parser object
 * @param type Argument type
 * @param name Argument name
 * @param alternateName Alternative argument name
 */
void argument_parser_add(argument_parser_t* obj, argument_type_t type, const char* name, const char* alternateName);

/**
 * @brief Parse the command line arguments
 * 
 * @note No need to modify the argc and argv variables
 * 
 * @param obj Parser object
 * @param argc Argument count
 * @param argv Arguments
 */
void argument_parser_parse(argument_parser_t* obj, int argc, const char** argv);

/**
 * @brief Get a parsed argument (if available)
 * 
 * @param obj Parser object
 * @param name Argument name (either normal or alternative one)
 */
argument_t* argument_parser_get(argument_parser_t* obj, const char* name);

/**
 * @brief Check if an argument is parsed/existent
 * 
 * @param obj Parser object
 * @param name Argument name (either normal or alternative one)
 */
uint8_t argument_parser_has(argument_parser_t* obj, const char* name);

#endif
