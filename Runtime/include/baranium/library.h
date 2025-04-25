#ifndef __BARANIUM__LIBRARY_H_
#define __BARANIUM__LIBRARY_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/backend/dynlibloader.h>
#include <baranium/variable.h>
#include <baranium/defines.h>
#include <baranium/script.h>
#include <stdint.h>

#define BARANIUM_LIBRARY_MAGIC_NUM0 'B'
#define BARANIUM_LIBRARY_MAGIC_NUM1 'G'
#define BARANIUM_LIBRARY_MAGIC_NUM2 'L'
#define BARANIUM_LIBRARY_MAGIC_NUM3 'F'

typedef struct baranium_library_header
{
    uint8_t magic[4];
    uint32_t version;
    uint64_t exports_count;
    uint64_t section_count;
} baranium_library_header;

typedef struct
{
    baranium_script_section_type_t type;
    index_t id;
    int num_params;
    baranium_variable_type_t return_type;
    size_t symnamelen;
    const char* symname;
} baranium_library_export;

typedef baranium_script_section baranium_library_section;

typedef struct baranium_library
{
    FILE* file;
    const char* path;
    const char* name;
    baranium_dynlib_handle dynlib;

    baranium_library_header libheader;
    baranium_library_export* exports;
    baranium_library_section* sections;
} baranium_library;

/**
 * @brief Load a library from a handle
 * 
 * @param path File path to library
 * @returns A library object
 */
BARANIUMAPI baranium_library* baranium_library_load(const char* path);

/**
 * @brief Dispose a library object
 * 
 * @param lib The library object
 */
BARANIUMAPI void baranium_library_dispose(baranium_library* lib);

/**
 * @brief Check if a library contains a given symbol
 * 
 * @param lib The library object
 * @param name Symbol name
 * @return 0 if not present, 1 if present
 */
BARANIUMAPI uint8_t baranium_library_has_symbol(baranium_library* lib, const char* name);

/**
 * @brief Get a field from a library
 * 
 * @note DISPOSE THE FIELD AFTERWARDS USING `baranium_field_dispose`,
 *       this will not delete the field from the library but rather
 *       clear the allocated memory
 * 
 * @param lib The library containing the field
 * @param name The name of the field
 * 
 * @returns The found field, `NULL` if not found
 */
BARANIUMAPI baranium_field* baranium_library_get_field(baranium_library* lib, const char* name);

/**
 * @brief Get a field from a library
 * 
 * @note DISPOSE THE FIELD AFTERWARDS USING `baranium_field_dispose`,
 *       this will not delete the field from the library but rather
 *       clear the allocated memory
 * 
 * @param lib The library containing the field
 * @param fieldID The ID of the field
 * 
 * @returns The found field, `NULL` if not found
 */
BARANIUMAPI baranium_field* baranium_library_get_field_by_id(baranium_library* lib, index_t fieldID);
 
/**
 * @brief Get a function from a library
 * 
 * @note DISPOSE THE FUNCTION AFTERWARDS USING `baranium_function_dispose`,
 *       this will not delete the function from the library but rather
 *       clear the allocated memory
 * 
 * @param lib The library containing the function
 * @param name The function name
 * 
 * @returns The found function, `NULL` if not found
 */
BARANIUMAPI baranium_function* baranium_library_get_function(baranium_library* lib, const char* name);

/**
 * @brief Get a function from a library
 * 
 * @param lib The library containing the function
 * @param functionID The function ID
 * 
 * @returns The found function, `NULL` if not found
 */
BARANIUMAPI baranium_function* baranium_library_get_function_by_id(baranium_library* lib, index_t functionID);

#ifdef __cplusplus
}
#endif

#endif
