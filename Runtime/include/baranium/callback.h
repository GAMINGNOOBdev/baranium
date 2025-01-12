#ifndef __BARANIUM__CALLBACK_H_
#define __BARANIUM__CALLBACK_H_ 1

#include "defines.h"
#include "variable.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*baranium_callback_t)(void** dataptr, baranium_variable_type_t* datatypes, int numData);

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
BARANIUMAPI baranium_callback_list_entry* baranium_callback_find_by_cb_ptr(baranium_callback_t* cb);

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
BARANIUMAPI void baranium_callback_remove_by_cb_ptr(baranium_callback_t* cb);

#ifdef __cplusplus
}
#endif

#endif
