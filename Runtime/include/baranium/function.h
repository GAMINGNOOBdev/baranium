#ifndef __BARANIUM__FUNCTION_H_
#define __BARANIUM__FUNCTION_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/variable.h>
#include <baranium/defines.h>

///////////////////////////////////////////////////////////
///                                                     ///
///   forward declarations needed to create a pointer   ///
///                                                     ///
///////////////////////////////////////////////////////////

struct baranium_script;
struct baranium_library;

typedef struct baranium_function
{
    index_t id;
    size_t data_size;
    uint8_t parameter_count;
    baranium_variable_type_t return_type;
    baranium_value_t return_value;
    void* data;
    struct baranium_script* script;
    struct baranium_library* library;
} baranium_function;

/**
 * @brief Dispose a function
 * 
 * @param var The function to dispose
 */
BARANIUMAPI void baranium_function_dispose(baranium_function* var);

/**
 * @brief Call a function
 *
 * @param function Function handle
 * @param dataptr An array of pointers pointing to variable values
 * @param datatypes An array of variable types
 * @param numData The number of input parameters
 */
BARANIUMAPI void baranium_function_call(baranium_function* function, baranium_value_t* dataptr, baranium_variable_type_t* datatypes, int numData);

#ifdef __cplusplus
}
#endif

#endif
