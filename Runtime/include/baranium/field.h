#ifndef __BARANIUM__FIELD_H_
#define __BARANIUM__FIELD_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "variable.h"

typedef struct BaraniumField
{
    enum BaraniumVariableType Type;
    void* Value;
    index_t ID;
} BaraniumField;

/**
 * @brief Dispose a field
 * 
 * @param var The field to dispose
 */
BARANIUMAPI void baranium_field_dispose(BaraniumField* var);

#ifdef __cplusplus
}
#endif

#endif