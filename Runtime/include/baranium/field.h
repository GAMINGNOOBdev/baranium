#ifndef __BARANIUM__FIELD_H_
#define __BARANIUM__FIELD_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "variable.h"

typedef struct BaraniumField
{
    enum BaraniumVariableType Type;
    size_t Size;
    void* Value;
    index_t ID;
} BaraniumField;

/**
 * @brief Dispose a field
 * 
 * @param field The field to dispose
 */
BARANIUMAPI void baranium_field_dispose(BaraniumField* field);

#ifdef __cplusplus
}
#endif

#endif