#ifndef __BARANIUM__CALLBACK_H_
#define __BARANIUM__CALLBACK_H_ 1

#include "defines.h"
#include "variable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BARANIUM_CALLBACK_INIT(data, expectedVariableMinimum, expectedVariableMaximum) \
    assert((data != NULL)); \
    baranium_value_t* dataptr = data->dataptr; \
    baranium_variable_type_t* datatypes = data->datatypes; \
    int num_data = data->num_data; \
    assert(num_data <= expectedVariableMaximum && num_data >= expectedVariableMinimum); \
    if (expectedVariableMinimum != 0) assert((dataptr != NULL) && (datatypes != NULL))

#define BARANIUM_CALLBACK_GET_VARIABLE(name, target_type, param_index) \
    baranium_compiled_variable name = {0,{0},0}; \
    if (datatypes[param_index] != BARANIUM_VARIABLE_TYPE_VOID && datatypes[param_index] != BARANIUM_VARIABLE_TYPE_INVALID) \
    {\
        size_t dataSize = baranium_variable_get_size_of_type(datatypes[param_index]); \
        if (dataSize == (size_t)-1) \
            dataSize = strlen(dataptr[param_index].ptr); \
        name = (baranium_compiled_variable){datatypes[param_index], dataptr[param_index], dataSize}; \
        baranium_compiled_variable_convert_to_type(&name, target_type); \
        dataptr[param_index] = name.value; \
        datatypes[param_index] = name.type; \
    }

#define BARANIUM_CALLBACK_GET_VARIABLE_VALUE(type_def, name, defaultvalue, target_type, param_index) \
    type_def name = defaultvalue; \
    baranium_compiled_variable variable_##name = {0,{0},0}; \
    if (datatypes[param_index] != BARANIUM_VARIABLE_TYPE_VOID && datatypes[param_index] != BARANIUM_VARIABLE_TYPE_INVALID) \
    {\
        size_t dataSize = baranium_variable_get_size_of_type(datatypes[param_index]); \
        if (dataSize == (size_t)-1) \
            dataSize = strlen(dataptr[param_index].ptr); \
        variable_##name = (baranium_compiled_variable){datatypes[param_index], dataptr[param_index], dataSize}; \
        baranium_compiled_variable_convert_to_type(&variable_##name, target_type); \
        dataptr[param_index] = variable_##name.value; \
        datatypes[param_index] = variable_##name.type; \
        if (variable_##name.type == BARANIUM_VARIABLE_TYPE_STRING) \
            name = (type_def)variable_##name.value.ptr;\
        else if (variable_##name.type == BARANIUM_VARIABLE_TYPE_FLOAT) \
            name = *(type_def*)&variable_##name.value.numfloat; \
        else \
            name = *(type_def*)&variable_##name.value.num64;\
    }

#define BARANIUM_CALLBACK_RETURN_VARIABLE(name) baranium_compiled_variable_push_to_stack(baranium_get_context()->cpu, &name);

typedef struct baranium_callback_data_list_t
{
    baranium_value_t* dataptr;
    baranium_variable_type_t* datatypes;
    int num_data;
} baranium_callback_data_list_t;

typedef void(*baranium_callback_t)(baranium_callback_data_list_t* callbackdata);

typedef struct baranium_callback_list_entry
{
    struct baranium_callback_list_entry* prev;
    index_t id;
    int numParams;
    baranium_callback_t callback;
    struct baranium_callback_list_entry* next;
} baranium_callback_list_entry;

typedef struct baranium_callback_list
{
    baranium_callback_list_entry* start;
    baranium_callback_list_entry* end;
    size_t count;
} baranium_callback_list;

typedef void(*internal_operation_t)(index_t id);

/**
 * @brief Set internal operation callbacks
 */
BARANIUMAPI void baranium_callback_set_internal_operations(internal_operation_t instantiateCB, internal_operation_t deleteCB, internal_operation_t attachCB, internal_operation_t detachCB);

/**
 * @brief Create a new callback list
 */
BARANIUMAPI baranium_callback_list* baranium_callback_list_init(void);

/**
 * @brief Dispose a callback list
 * 
 * @param list The list that will be disposed
 */
BARANIUMAPI void baranium_callback_list_dispose(baranium_callback_list* list);

/**
 * @brief Add a callback to a C function
 * 
 * @param id Callback ID
 * @param cb Callback pointer
 * @param numParams Number of parameters
 */
BARANIUMAPI void baranium_callback_add(index_t id, baranium_callback_t cb, int numParams);

/**
 * @brief Find a callback entry
 * 
 * @param id Callback ID
 * 
 * @returns The list entry of the callback
 */
BARANIUMAPI baranium_callback_list_entry* baranium_callback_find_by_id(index_t id);

/**
 * @brief Find a callback entry
 * 
 * @param cb Callback pointer
 * 
 * @returns The list entry of the callback
 */
BARANIUMAPI baranium_callback_list_entry* baranium_callback_find_by_cb_ptr(baranium_callback_t cb);

/**
 * @brief Remove a callback
 * 
 * @param id Callback ID
 */
BARANIUMAPI void baranium_callback_remove_by_id(index_t id);

/**
 * @brief Remove a callback
 * 
 * @param cb Callback pointer
 */
BARANIUMAPI void baranium_callback_remove_by_cb_ptr(baranium_callback_t cb);

#ifdef __cplusplus
}
#endif

#endif
