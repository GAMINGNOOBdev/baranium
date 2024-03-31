#ifndef __BARANIUM__VARIABLE_H_
#define __BARANIUM__VARIABLE_H_ 1

#include "defines.h"

enum BaraniumVariableType
{
    Invalid = -1,
    Void,
    GameObject,
    String,
    Float,
    Bool,
    Int,
    Uint,
};

typedef struct BaraniumVariable
{
    enum BaraniumVariableType Type;
    void* Value;
    char* Name;
    int ID;
} BaraniumVariable;

#endif