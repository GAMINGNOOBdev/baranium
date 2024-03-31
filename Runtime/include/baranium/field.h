#ifndef __BARANIUM__FIELD_H_
#define __BARANIUM__FIELD_H_ 1

#include "variable.h"

typedef struct BaraniumField
{
    enum BaraniumVariableType Type;
    void* Value;
    char* Name;
    int ID;
} BaraniumField;

#endif