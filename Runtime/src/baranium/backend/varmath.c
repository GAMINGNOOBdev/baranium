#include <baranium/backend/varmath.h>
#include <baranium/backend/errors.h>
#include <baranium/cpu/bstack.h>
#include <baranium/runtime.h>
#include <baranium/logging.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>

baranium_compiled_variable* baranium_compiled_variable_pop_from_stack(bcpu* cpu)
{
    if (cpu == NULL)
        return NULL;

    baranium_compiled_variable* result = (baranium_compiled_variable*)malloc(sizeof(baranium_compiled_variable));
    if (!result)
        return NULL;

    memset(result, 0, sizeof(baranium_compiled_variable));
    result->type = (enum BaraniumVariableType)bstack_pop(cpu->stack);
    size_t size = result->size = (size_t)bstack_pop(cpu->stack);
    void* value = result->value = malloc(result->size);
    if (value == NULL)
    {
        free(result);
        return NULL;
    }

    memset(result->value, 0, result->size);

    uint64_t data = 0;

    if (size <= 8)
    {
        data = bstack_pop(cpu->stack);
        memcpy(value, &data, size);
    }
    else
    {
        size_t leftOverSize = size - size % 8;
        data = bstack_pop(cpu->stack);
        void* valPtr = (void*)(((uint64_t)value) + leftOverSize);
        leftOverSize-=8;
        memcpy(valPtr, &data, size % 8);
        for (; leftOverSize >= 0; leftOverSize-=8)
        {
            data = bstack_pop(cpu->stack);
            valPtr = (void*)(((uint64_t)value) + leftOverSize);
            memcpy(valPtr, &data, 8);
            if (leftOverSize == 0)
                break;
        }
        leftOverSize=0;
    }

    return result;
}

void baranium_compiled_variable_push_to_stack(bcpu* cpu, baranium_compiled_variable* var)
{
    enum BaraniumVariableType type = var->type;
    uint64_t size = var->size;
    void* value = var->value;

    uint64_t data = 0;
    if (size <= 8)
    {
        memcpy(&data, value, size);
        bstack_push(cpu->stack, data);
    }
    else
    {
        size_t leftOverSize = size;
        int index = 0;
        void* valPtr;
        for (index = 0; index * 8 < size; index++)
        {
            if (leftOverSize < 8)
                break;
            valPtr = (void*)((uint64_t)value + index*8);
            memcpy(&data, valPtr, 8);
            leftOverSize -= 8;
            bstack_push(cpu->stack, data);
        }
        valPtr = (void*)((uint64_t)value + index*8);
        data = 0;
        memcpy(&data, valPtr, leftOverSize);
        bstack_push(cpu->stack, data);
    }

    bstack_push(cpu->stack, (uint64_t)size);
    bstack_push(cpu->stack, (uint64_t)type);
}

void baranium_compiled_variable_dispose(baranium_compiled_variable* varptr)
{
    if (!varptr)
        return;

    free(varptr);
}

#define PerformOperation(out, exprl, exprr, what) out = exprl what exprr
#define ForeachOperation(operation, out, exprl, exprr) \
        if(operation == BARANIUM_VARIABLE_OPERATION_MOD) if (exprr == 0 && baranium_get_context() != NULL) \
                                                         {\
                                                            baranium_get_context()->cpu->killTriggered = 1;\
                                                            baranium_get_context()->cpu->flags.FORCED_KILL = 1;\
                                                            bstack_push(baranium_get_context()->cpu->stack, ERR_DIV_BY_ZERO);\
                                                         } \
                                                         else PerformOperation(out, exprl, exprr, %); \
        if(operation == BARANIUM_VARIABLE_OPERATION_DIV) if (exprr == 0 && baranium_get_context() != NULL) \
                                                         {\
                                                            baranium_get_context()->cpu->killTriggered = 1;\
                                                            baranium_get_context()->cpu->flags.FORCED_KILL = 1;\
                                                            bstack_push(baranium_get_context()->cpu->stack, ERR_DIV_BY_ZERO);\
                                                         } \
                                                         else PerformOperation(out, exprl, exprr, /); \
        if(operation == BARANIUM_VARIABLE_OPERATION_MUL) PerformOperation(out, exprl, exprr, *); \
        if(operation == BARANIUM_VARIABLE_OPERATION_SUB) PerformOperation(out, exprl, exprr, -); \
        if(operation == BARANIUM_VARIABLE_OPERATION_ADD) PerformOperation(out, exprl, exprr, +); \
        if(operation == BARANIUM_VARIABLE_OPERATION_AND) PerformOperation(out, exprl, exprr, &); \
        if(operation == BARANIUM_VARIABLE_OPERATION_OR) PerformOperation(out, exprl, exprr, |); \
        if(operation == BARANIUM_VARIABLE_OPERATION_XOR) PerformOperation(out, exprl, exprr, ^); \
        if(operation == BARANIUM_VARIABLE_OPERATION_SHFTL) PerformOperation(out, exprl, exprr, <<); \
        if(operation == BARANIUM_VARIABLE_OPERATION_SHFTR) PerformOperation(out, exprl, exprr, >>)

void baranium_compiled_variable_perform_arithmetic_operation(void* dataLeft, void* dataRight, enum BaraniumVariableType target, uint8_t operation)
{
    if (target == BaraniumVariableType_Invalid || target == BaraniumVariableType_Void || target == BaraniumVariableType_String)
        return;

    if (target == BaraniumVariableType_Object)
    {
        ForeachOperation(operation, *((int64_t*)dataLeft), *((int64_t*)dataLeft), *((int64_t*)dataRight));
        return;
    }

    if (target == BaraniumVariableType_Float)
    {
        if(operation == BARANIUM_VARIABLE_OPERATION_AND || operation == BARANIUM_VARIABLE_OPERATION_OR ||
           operation == BARANIUM_VARIABLE_OPERATION_XOR || operation == BARANIUM_VARIABLE_OPERATION_SHFTL ||
           operation == BARANIUM_VARIABLE_OPERATION_SHFTR)
        {
            LOGERROR("Illegal operation: tried bitwise operations on float");
            return;
        }

        if(operation == BARANIUM_VARIABLE_OPERATION_MOD)
        {
            LOGERROR("Illegal operation: tried to mod a float (wtf are you doing)");
            return;
        }

        if(operation == BARANIUM_VARIABLE_OPERATION_DIV)
            PerformOperation(*((float*)dataLeft), *((float*)dataLeft), *((float*)dataRight), /);
        if(operation == BARANIUM_VARIABLE_OPERATION_MUL)
            PerformOperation(*((float*)dataLeft), *((float*)dataLeft), *((float*)dataRight), *);
        if(operation == BARANIUM_VARIABLE_OPERATION_SUB)
            PerformOperation(*((float*)dataLeft), *((float*)dataLeft), *((float*)dataRight), -);
        if(operation == BARANIUM_VARIABLE_OPERATION_ADD)
            PerformOperation(*((float*)dataLeft), *((float*)dataLeft), *((float*)dataRight), +);

        return;
    }

    if (target == BaraniumVariableType_Bool)
    {
        ForeachOperation(operation, *((uint8_t*)dataLeft), *((uint8_t*)dataLeft), *((uint8_t*)dataRight));
        return;
    }

    if (target == BaraniumVariableType_Int)
    {
        ForeachOperation(operation, *((int32_t*)dataLeft), *((int32_t*)dataLeft), *((int32_t*)dataRight));
        return;
    }

    if (target == BaraniumVariableType_Uint)
    {
        ForeachOperation(operation, *((uint32_t*)dataLeft), *((uint32_t*)dataLeft), *((uint32_t*)dataRight));
        return;
    }
}

void baranium_compiled_variable_as_object(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(BaraniumVariableType_Object);
    int64_t* newVal = malloc(size);
    memset(newVal, 0, size);

    if (var->type == BaraniumVariableType_Float)
        (*newVal) = (int64_t)*((float*)var->value);
    
    if (var->type == BaraniumVariableType_Int)
        (*newVal) = (int64_t)*((int32_t*)var->value);

    if (var->type == BaraniumVariableType_Uint)
        (*newVal) = (int64_t)*((uint32_t*)var->value);

    free(var->value);
    var->value = newVal;
    var->size = size;
}

void baranium_compiled_variable_as_float(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(BaraniumVariableType_Float);
    float* newVal = malloc(size);
    memset(newVal, 0, size);

    if (var->type == BaraniumVariableType_Int)
        (*newVal) = (float)*((int32_t*)var->value);

    if (var->type == BaraniumVariableType_Uint)
        (*newVal) = (float)*((uint32_t*)var->value);

    free(var->value);
    var->value = newVal;
    var->size = size;
}

void baranium_compiled_variable_as_bool(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(BaraniumVariableType_Bool);
    uint8_t* newVal = malloc(size);
    memset(newVal, 0, size);

    if (var->type == BaraniumVariableType_Object)
        (*newVal) = 0 < *((int64_t*)var->value);

    if (var->type == BaraniumVariableType_Float)
        (*newVal) = 0 < *((float*)var->value);
    
    if (var->type == BaraniumVariableType_Int)
        (*newVal) = 0 < *((int32_t*)var->value);

    if (var->type == BaraniumVariableType_Uint)
        (*newVal) = 0 < *((uint32_t*)var->value);

    free(var->value);
    var->value = newVal;
    var->size = size;
}

void baranium_compiled_variable_as_int(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(BaraniumVariableType_Int);
    int32_t* newVal = malloc(size);
    memset(newVal, 0, size);

    if (var->type == BaraniumVariableType_Object)
        (*newVal) = (int32_t)*((int64_t*)var->value);

    if (var->type == BaraniumVariableType_Bool)
        (*newVal) = (int32_t)*((uint8_t*)var->value);

    if (var->type == BaraniumVariableType_Float)
        (*newVal) = (int32_t)*((float*)var->value);

    if (var->type == BaraniumVariableType_Uint)
        (*newVal) = (int32_t)*((uint32_t*)var->value);

    free(var->value);
    var->value = newVal;
    var->size = size;
}

void baranium_compiled_variable_as_uint(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(BaraniumVariableType_Uint);
    uint32_t* newVal = malloc(size);
    memset(newVal, 0, size);

    if (var->type == BaraniumVariableType_Object)
        (*newVal) = (uint32_t)*((int64_t*)var->value);

    if (var->type == BaraniumVariableType_Bool)
        (*newVal) = (uint32_t)*((uint8_t*)var->value);

    if (var->type == BaraniumVariableType_Float)
        (*newVal) = (uint32_t)*((float*)var->value);
    
    if (var->type == BaraniumVariableType_Int)
        (*newVal) = (uint32_t)*((int32_t*)var->value);

    if (var->type == BaraniumVariableType_Uint)
        (*newVal) = (uint32_t)*((uint32_t*)var->value);

    free(var->value);
    var->value = newVal;
    var->size = size;
}

void baranium_compiled_variable_convert_to_type(baranium_compiled_variable* var, enum BaraniumVariableType targetType)
{
    if (targetType == BaraniumVariableType_Invalid || targetType == BaraniumVariableType_Void || targetType == BaraniumVariableType_String) // strings should already be covered
        return;

    if (targetType == BaraniumVariableType_Object)
        baranium_compiled_variable_as_object(var);

    if (targetType == BaraniumVariableType_Float)
        baranium_compiled_variable_as_float(var);

    if (targetType == BaraniumVariableType_Bool)
        baranium_compiled_variable_as_bool(var);

    if (targetType == BaraniumVariableType_Int)
        baranium_compiled_variable_as_int(var);

    if (targetType == BaraniumVariableType_Uint)
        baranium_compiled_variable_as_uint(var);
}

void baranium_compiled_variable_combine(baranium_compiled_variable* lhs, baranium_compiled_variable* rhs, uint8_t operation, enum BaraniumVariableType type)
{
    if (lhs == NULL || rhs == NULL || operation == BARANIUM_VARIABLE_OPERATION_NONE)
        return;

    enum BaraniumVariableType resultType = type;
    if (type == BaraniumVariableType_Invalid || type == BaraniumVariableType_Void)
        resultType = lhs->type;
    
    if (resultType == BaraniumVariableType_Invalid || resultType == BaraniumVariableType_Void)
        return;
    
    if (operation != BARANIUM_VARIABLE_OPERATION_ADD && resultType == BaraniumVariableType_String) // also special case
    {
        LOGERROR("Strings can only be combined using '+'");
        return;
    }

    if (operation == BARANIUM_VARIABLE_OPERATION_ADD && resultType == BaraniumVariableType_String) // very special case
    {
        void* value = stringf("%s%s", baranium_variable_stringify(lhs->type, lhs->value), baranium_variable_stringify(rhs->type, rhs->value));
        free(lhs->value);
        size_t size = strlen(value)+1;
        lhs->value = malloc(size);
        memset(lhs->value, 0, size);
        memcpy(lhs->value, value, size-1);
        lhs->type = BaraniumVariableType_String;
        return;
    }

    if (rhs->type != lhs->type)
        baranium_compiled_variable_convert_to_type(rhs, lhs->type);

    baranium_compiled_variable_perform_arithmetic_operation(lhs->value, rhs->value, lhs->type, operation);

    if (lhs->type != resultType)
        baranium_compiled_variable_convert_to_type(lhs, resultType);
}