#ifndef __BARANIUM__FIELD_H_
#define __BARANIUM__FIELD_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "variable.h"

typedef struct baranium_field
{
    baranium_variable_type_t type;
    size_t size;
    baranium_value_t value;
    index_t id;
} baranium_field;

/**
 * @brief Set the value of a field
 *
 * @note converts the given value to the fields type
 *
 * @param field Field whose value will be set
 * @param value Value pointer
 * @param size Size of the value (only used when the given value is a string)
 * @param type Type of the given value (has to be specified)
 */
BARANIUMAPI void baranium_field_set_value(baranium_field* field, void* value, size_t size, baranium_variable_type_t type);

/**
 * @brief Get the value of a field
 *
 * @note This function converts the field's value only for the output and not for the field itself
 * 
 * @param field Field whose value will be get
 * @param outputType Type of the output, variable will be converted to the given output type unless the type is invalid, in which case the field's type is used
 * @returns Output data
 */
BARANIUMAPI baranium_value_t baranium_field_get_value(baranium_field* field, baranium_variable_type_t outputType);

/**
 * @brief Dispose a field
 * 
 * @param field The field to dispose
 */
BARANIUMAPI void baranium_field_dispose(baranium_field* field);

#ifdef __cplusplus
}
#endif

#endif
