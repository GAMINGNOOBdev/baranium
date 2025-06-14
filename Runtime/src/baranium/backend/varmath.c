#include <baranium/backend/varmath.h>
#include <baranium/backend/errors.h>
#include <baranium/cpu/bstack.h>
#include <baranium/variable.h>
#include <baranium/defines.h>
#include <baranium/runtime.h>
#include <baranium/logging.h>
#include <baranium/bcpu.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_APPLE
#   include <stdlib.h>
#else
#   include <malloc.h>
#endif

baranium_compiled_variable* baranium_compiled_variable_pop_from_stack(bcpu* cpu)
{
    if (cpu == NULL)
        return NULL;

    baranium_compiled_variable* result = (baranium_compiled_variable*)malloc(sizeof(baranium_compiled_variable));
    if (!result)
        return NULL;

    memset(result, 0, sizeof(baranium_compiled_variable));
    
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, result);

    return result;
}

void baranium_compiled_variable_pop_from_stack_into_variable(bcpu* cpu, baranium_compiled_variable* output)
{
    if (cpu == NULL)
        return;

    if (!output)
        return;

    baranium_variable_type_t oldType = output->type;
    output->type = (baranium_variable_type_t)bstack_pop(cpu->stack);
    size_t size = output->size = (size_t)bstack_pop(cpu->stack);
    if (output->type == BARANIUM_VARIABLE_TYPE_STRING)
    {
        if (output->type != oldType)
            output->value.ptr = malloc(output->size+1);
        else
            output->value.ptr = realloc(output->value.ptr, output->size+1);

        if (output->value.ptr == NULL)
        {
            bstack_push(cpu->stack, BARANIUM_ERROR_OUT_OF_MEMORY);
            cpu->flags.FORCED_KILL = 1;
            cpu->kill_triggered = 1;
            return;
        }

        ((uint8_t*)output->value.ptr)[output->size] = 0;
    }

    uint64_t data = 0;

    void* basevalue = &output->value.num64;
    if (output->type == BARANIUM_VARIABLE_TYPE_STRING)
        basevalue = output->value.ptr;

    if (size <= 8)
    {
        data = bstack_pop(cpu->stack);
        memcpy(basevalue, &data, size);
    }
    else
    {
        size_t leftOverSize = size - size % 8;
        void* valPtr = (void*)(((uint64_t)basevalue) + leftOverSize);
        if (size % 8 != 0)
        {
            data = bstack_pop(cpu->stack);
            memcpy(valPtr, &data, size % 8);
        }
        size_t count = leftOverSize/8;
        leftOverSize-=8;
        for (size_t i = 0; i < count; i++)
        {
            data = bstack_pop(cpu->stack);
            valPtr = (void*)(((uint64_t)basevalue) + leftOverSize);
            memcpy(valPtr, &data, 8);
            leftOverSize-=8;
        }
        leftOverSize=0;
    }

    ///TODO: fix invalid indexes and stuff

    if (output->type == BARANIUM_VARIABLE_TYPE_STRING)
        output->size++;
}

void baranium_compiled_variable_push_to_stack(bcpu* cpu, baranium_compiled_variable* var)
{
    baranium_variable_type_t type = var->type;
    uint64_t size = var->size;
    void* value = &var->value.num64;
    if (type == BARANIUM_VARIABLE_TYPE_STRING)
        value = var->value.ptr;

    uint64_t data = 0;
    if (size <= 8)
    {
        memcpy(&data, value, size);
        bstack_push(cpu->stack, data);
    }
    else
    {

        size_t leftOverSize = size;
        size_t index = 0;
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
        if(operation == BARANIUM_VARIABLE_OPERATION_MOD) { if (exprr == 0 && baranium_get_runtime() != NULL) \
                                                         {\
                                                            baranium_get_runtime()->cpu->kill_triggered = 1;\
                                                            baranium_get_runtime()->cpu->flags.FORCED_KILL = 1;\
                                                            bstack_push(baranium_get_runtime()->cpu->stack, BARANIUM_ERROR_DIV_BY_ZERO);\
                                                         } \
                                                         else PerformOperation(out, exprl, exprr, %); } \
        if(operation == BARANIUM_VARIABLE_OPERATION_DIV) { if (exprr == 0 && baranium_get_runtime() != NULL) \
                                                         {\
                                                            baranium_get_runtime()->cpu->kill_triggered = 1;\
                                                            baranium_get_runtime()->cpu->flags.FORCED_KILL = 1;\
                                                            bstack_push(baranium_get_runtime()->cpu->stack, BARANIUM_ERROR_DIV_BY_ZERO);\
                                                         } \
                                                         else PerformOperation(out, exprl, exprr, /); } \
        if(operation == BARANIUM_VARIABLE_OPERATION_MUL) PerformOperation(out, exprl, exprr, *); \
        if(operation == BARANIUM_VARIABLE_OPERATION_SUB) PerformOperation(out, exprl, exprr, -); \
        if(operation == BARANIUM_VARIABLE_OPERATION_ADD) PerformOperation(out, exprl, exprr, +); \
        if(operation == BARANIUM_VARIABLE_OPERATION_AND) PerformOperation(out, exprl, exprr, &); \
        if(operation == BARANIUM_VARIABLE_OPERATION_OR) PerformOperation(out, exprl, exprr, |); \
        if(operation == BARANIUM_VARIABLE_OPERATION_XOR) PerformOperation(out, exprl, exprr, ^); \
        if(operation == BARANIUM_VARIABLE_OPERATION_SHFTL) PerformOperation(out, exprl, exprr, <<); \
        if(operation == BARANIUM_VARIABLE_OPERATION_SHFTR) PerformOperation(out, exprl, exprr, >>)

void baranium_compiled_variable_perform_arithmetic_operation(baranium_value_t* dataLeft, baranium_value_t* dataRight, baranium_variable_type_t target, uint8_t operation)
{
    if (target == BARANIUM_VARIABLE_TYPE_INVALID || target == BARANIUM_VARIABLE_TYPE_VOID || target == BARANIUM_VARIABLE_TYPE_STRING)
        return;

    if (target == BARANIUM_VARIABLE_TYPE_OBJECT)
    {
        ForeachOperation(operation, dataLeft->num64, dataLeft->num64, dataRight->num64);
        return;
    }

    if (target == BARANIUM_VARIABLE_TYPE_FLOAT)
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
            PerformOperation(dataLeft->numfloat, dataLeft->numfloat, dataRight->numfloat, /);
        if(operation == BARANIUM_VARIABLE_OPERATION_MUL)
            PerformOperation(dataLeft->numfloat, dataLeft->numfloat, dataRight->numfloat, *);
        if(operation == BARANIUM_VARIABLE_OPERATION_SUB)
            PerformOperation(dataLeft->numfloat, dataLeft->numfloat, dataRight->numfloat, -);
        if(operation == BARANIUM_VARIABLE_OPERATION_ADD)
            PerformOperation(dataLeft->numfloat, dataLeft->numfloat, dataRight->numfloat, +);

        return;
    }

    if (target == BARANIUM_VARIABLE_TYPE_DOUBLE)
    {
        if(operation == BARANIUM_VARIABLE_OPERATION_AND || operation == BARANIUM_VARIABLE_OPERATION_OR ||
           operation == BARANIUM_VARIABLE_OPERATION_XOR || operation == BARANIUM_VARIABLE_OPERATION_SHFTL ||
           operation == BARANIUM_VARIABLE_OPERATION_SHFTR)
        {
            LOGERROR("Illegal operation: tried bitwise operations on double");
            return;
        }

        if(operation == BARANIUM_VARIABLE_OPERATION_MOD)
        {
            LOGERROR("Illegal operation: tried to mod a double (wtf are you doing)");
            return;
        }

        if(operation == BARANIUM_VARIABLE_OPERATION_DIV)
            PerformOperation(dataLeft->numdouble, dataLeft->numdouble, dataRight->numdouble, /);
        if(operation == BARANIUM_VARIABLE_OPERATION_MUL)
            PerformOperation(dataLeft->numdouble, dataLeft->numdouble, dataRight->numdouble, *);
        if(operation == BARANIUM_VARIABLE_OPERATION_SUB)
            PerformOperation(dataLeft->numdouble, dataLeft->numdouble, dataRight->numdouble, -);
        if(operation == BARANIUM_VARIABLE_OPERATION_ADD)
            PerformOperation(dataLeft->numdouble, dataLeft->numdouble, dataRight->numdouble, +);

        return;
    }

    if (target == BARANIUM_VARIABLE_TYPE_BOOL)
    {
        ForeachOperation(operation, dataLeft->num8, dataLeft->num8, dataRight->num8);
        return;
    }

    if (target == BARANIUM_VARIABLE_TYPE_INT32)
    {
        ForeachOperation(operation, dataLeft->snum32, dataLeft->snum32, dataRight->snum32);
        return;
    }

    if (target == BARANIUM_VARIABLE_TYPE_UINT32)
    {
        ForeachOperation(operation, dataLeft->num32, dataLeft->num32, dataRight->num32);
        return;
    }

    if (target == BARANIUM_VARIABLE_TYPE_INT8)
    {
        ForeachOperation(operation, dataLeft->snum8, dataLeft->snum8, dataRight->snum8);
        return;
    }

    if (target == BARANIUM_VARIABLE_TYPE_UINT8)
    {
        ForeachOperation(operation, dataLeft->num8, dataLeft->num8, dataRight->num8);
        return;
    }

    if (target == BARANIUM_VARIABLE_TYPE_INT16)
    {
        ForeachOperation(operation, dataLeft->snum16, dataLeft->snum16, dataRight->snum16);
        return;
    }

    if (target == BARANIUM_VARIABLE_TYPE_UINT16)
    {
        ForeachOperation(operation, dataLeft->num16, dataLeft->num16, dataRight->num16);
        return;
    }

    if (target == BARANIUM_VARIABLE_TYPE_INT64)
    {
        ForeachOperation(operation, dataLeft->snum64, dataLeft->snum64, dataRight->snum64);
        return;
    }

    if (target == BARANIUM_VARIABLE_TYPE_UINT64)
    {
        ForeachOperation(operation, dataLeft->num64, dataLeft->num64, dataRight->num64);
        return;
    }
}

static const char* string_ptr_tmp = NULL;
static int64_t string_to_num_tmp = 0;
static double string_to_num_float_tmp = 0.0;
static uint8_t string_to_num_is_float = 0;
static uint8_t string_to_num_is_negative = 0;

uint8_t string_is_number(const char* str)
{
    string_to_num_is_negative = 0;
    string_to_num_float_tmp = 0;
    string_to_num_is_float = 0;
    string_to_num_tmp = 0;
    string_ptr_tmp = str;

    size_t len = strlen(str);
    size_t index = 0;
    char c = str[index];
    uint16_t decimalPlaces = 1;

    if (c == '+' || c == '-')
    {
        string_to_num_is_negative = c == '-';
        index++;
    }

    // check for hex/binary formats beforehand
    if (len-index > 2)
    {
        c = str[index+1];
        if (c == 'x' || c == 'X')
            goto parseHex;

        if (c == 'b' || c == 'B')
            goto parseBinary;
    }

    // check for '.' aka a floating point
    for (size_t i = index; i < len && !string_to_num_is_float; i++)
        string_to_num_is_float = str[index] == '.';

    if (!string_to_num_is_float)
        goto parseInteger;

    for (; index < len; index++)
    {
        string_to_num_tmp *= 10;

        c = str[index];
        if (c == '.')
        {
            index++;
            break;
        }

        if ((c > '0' && c < '9'))
            string_to_num_tmp += c-'0';
        else return 0;
    }
    for (; index < len; index++)
    {
        decimalPlaces*=10;
        string_to_num_float_tmp*=10;
        string_to_num_float_tmp += c-'0'; 
    }
    string_to_num_float_tmp /= decimalPlaces;
    string_to_num_float_tmp += string_to_num_tmp;
    string_to_num_float_tmp *= string_to_num_is_negative ? -1 : 1;
    return 1;

parseInteger:
    for (; index < len; index++)
    {
        string_to_num_tmp *= 10;

        c = str[index];

        if (c >= '0' && c <= '9')
            string_to_num_tmp += c-'0';
        else return 0;
    }
    string_to_num_tmp *= string_to_num_is_negative ? -1 : 1;
    string_to_num_float_tmp = string_to_num_tmp;

    return 1;

parseHex:

    index = 0;
    c = str[index];
    if (c != '0')
        return 0;
    index = 2;

    for (; index < len; index++)
    {
        string_to_num_tmp <<= 4;

        c = str[index];

        if ((c >= '0' && c <= '9'))
            string_to_num_tmp |= c-'0';
        else if((c >= 'a' && c <= 'f'))
            string_to_num_tmp |= 10 + c - 'a';
        else if((c >= 'A' || c <= 'F'))
            string_to_num_tmp |= 10 + c - 'A';
        else return 0;
    }
    string_to_num_tmp *= string_to_num_is_negative ? -1 : 1;
    string_to_num_float_tmp = string_to_num_tmp;

    return 1;

parseBinary:

    index = 0;
    c = str[index];
    if (c != '0')
        return 0;
    index = 2;

    for (; index < len; index++)
    {
        string_to_num_tmp <<= 1;

        c = str[index];

        if (c != '0' && c != '1')
            return 0;

        if (c == '0')
            continue;

        string_to_num_tmp |= 1;
    }
    string_to_num_tmp *= string_to_num_is_negative ? -1 : 1;
    string_to_num_float_tmp = string_to_num_tmp;

    return 1;
}

int64_t string_to_number(const char* str)
{
    if (str != string_ptr_tmp)
        string_is_number(str);

    return string_to_num_tmp;
}

float string_to_float(const char* str)
{
    if (str != string_ptr_tmp)
        string_is_number(str);

    return string_to_num_float_tmp;
}

double string_to_double(const char* str)
{
    if (str != string_ptr_tmp)
        string_is_number(str);

    return string_to_num_float_tmp;
}

int baranium_compiled_variable_as_object(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(BARANIUM_VARIABLE_TYPE_OBJECT);
    int64_t val = 0;
    int result = 1;

    if (var->type == BARANIUM_VARIABLE_TYPE_INT32)
        val = (int64_t)var->value.snum32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT32)
        val = (int64_t)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_STRING && string_is_number(var->value.str))
        val = (int64_t)string_to_number(var->value.str);
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT8)
        val = (int64_t)var->value.snum8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT8)
        val = (int64_t)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT16)
        val = (int64_t)var->value.snum16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT16)
        val = (int64_t)var->value.num16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT64)
        val = (int64_t)var->value.snum64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT64)
        val = (int64_t)var->value.num64;
    else result = 0;

    var->value.num64 = val;
    var->size = size;
    return result;
}

int baranium_compiled_variable_as_float(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(BARANIUM_VARIABLE_TYPE_FLOAT);
    float val = 0;
    int result = 1;

    if (var->type == BARANIUM_VARIABLE_TYPE_INT32)
        val = (float)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT32)
        val = (float)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_STRING && string_is_number(var->value.str))
        val = (float)string_to_float(var->value.str);
    else if (var->type == BARANIUM_VARIABLE_TYPE_DOUBLE)
        val = (float)var->value.numdouble;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT8)
        val = (float)var->value.snum8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT8)
        val = (float)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT16)
        val = (float)var->value.snum16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT16)
        val = (float)var->value.num16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT64)
        val = (float)var->value.snum64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT64)
        val = (float)var->value.num64;
    else result = 0;

    var->value.numfloat = val;
    var->size = size;
    return result;
}

int baranium_compiled_variable_as_double(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(BARANIUM_VARIABLE_TYPE_DOUBLE);
    double val = 0;
    int result = 1;

    if (var->type == BARANIUM_VARIABLE_TYPE_INT32)
        val = (double)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT32)
        val = (double)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_STRING && string_is_number(var->value.str))
        val = (double)string_to_double(var->value.str);
    else if (var->type == BARANIUM_VARIABLE_TYPE_FLOAT)
        val = (double)var->value.numfloat;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT8)
        val = (double)var->value.snum8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT8)
        val = (double)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT16)
        val = (double)var->value.snum16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT16)
        val = (double)var->value.num16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT64)
        val = (double)var->value.snum64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT64)
        val = (double)var->value.num64;
    else result = 0;

    var->value.numdouble = val;
    var->size = size;
    return result;
}

int baranium_compiled_variable_as_bool(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(BARANIUM_VARIABLE_TYPE_BOOL);
    uint8_t val = 0;
    int result = 1;

    if (var->type == BARANIUM_VARIABLE_TYPE_OBJECT)
        val = 0 < var->value.num64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_FLOAT)
        val = 0 < var->value.numfloat;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT32)
        val = 0 < var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT32)
        val = 0 < var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_STRING && string_is_number(var->value.str))
        val = 0 < string_to_number(var->value.str);
    else if (var->type == BARANIUM_VARIABLE_TYPE_STRING)
        val = 0 < (size_t)var->value.ptr;
    else if (var->type == BARANIUM_VARIABLE_TYPE_DOUBLE)
        val = 0 < var->value.numdouble;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT8)
        val = 0 < var->value.snum8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT8)
        val = 0 < var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT16)
        val = 0 < var->value.snum16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT16)
        val = 0 < var->value.num16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT64)
        val = 0 < var->value.snum64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT64)
        val = 0 < var->value.num64;
    else result = 0;

    var->value.num8 = val;
    var->size = size;
    return result;
}

int baranium_compiled_variable_as_int(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(BARANIUM_VARIABLE_TYPE_INT32);
    int32_t val = 0;
    int result = 1;

    if (var->type == BARANIUM_VARIABLE_TYPE_OBJECT)
        val = (int32_t)var->value.num64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_BOOL)
        val = (int32_t)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_FLOAT)
        val = (int32_t)var->value.numfloat;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT32)
        val = (int32_t)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_STRING && string_is_number(var->value.str))
        val = (int32_t)string_to_number(var->value.str);
    else if (var->type == BARANIUM_VARIABLE_TYPE_DOUBLE)
        val = (int32_t)var->value.numdouble;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT8)
        val = (int32_t)var->value.snum8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT8)
        val = (int32_t)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT16)
        val = (int32_t)var->value.snum16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT16)
        val = (int32_t)var->value.num16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT64)
        val = (int32_t)var->value.snum64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT64)
        val = (int32_t)var->value.num64;
    else result = 0;

    var->value.snum32 = val;
    var->size = size;
    return result;
}

int baranium_compiled_variable_as_uint(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(BARANIUM_VARIABLE_TYPE_UINT32);
    uint32_t val = 0;
    int result = 1;

    if (var->type == BARANIUM_VARIABLE_TYPE_OBJECT)
        val = (uint32_t)var->value.num64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_BOOL)
        val = (uint32_t)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_FLOAT)
        val = (uint32_t)var->value.numfloat;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT32)
        val = (uint32_t)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_STRING && string_is_number(var->value.str))
        val = (uint32_t)string_to_number(var->value.str);
    else if (var->type == BARANIUM_VARIABLE_TYPE_DOUBLE)
        val = (uint32_t)var->value.numdouble;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT8)
        val = (uint32_t)var->value.snum8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT8)
        val = (uint32_t)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT16)
        val = (uint32_t)var->value.snum16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT16)
        val = (uint32_t)var->value.num16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT64)
        val = (uint32_t)var->value.snum64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT64)
        val = (uint32_t)var->value.num64;
    else result = 0;

    var->value.num32 = val;
    var->size = size;
    return result;
}

int baranium_compiled_variable_as_byte(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(BARANIUM_VARIABLE_TYPE_INT8);
    int8_t val = 0;
    int result = 1;

    if (var->type == BARANIUM_VARIABLE_TYPE_OBJECT)
        val = (int8_t)var->value.num64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_BOOL)
        val = (int8_t)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_FLOAT)
        val = (int8_t)var->value.numfloat;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT32)
        val = (int8_t)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT32)
        val = (int8_t)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_STRING && string_is_number(var->value.str))
        val = (int8_t)string_to_number(var->value.str);
    else if (var->type == BARANIUM_VARIABLE_TYPE_DOUBLE)
        val = (int8_t)var->value.numdouble;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT8)
        val = (int8_t)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT16)
        val = (int8_t)var->value.snum16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT16)
        val = (int8_t)var->value.num16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT64)
        val = (int8_t)var->value.snum64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT64)
        val = (int8_t)var->value.num64;
    else result = 0;

    var->value.snum8 = val;
    var->size = size;
    return result;
}

int baranium_compiled_variable_as_ubyte(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(BARANIUM_VARIABLE_TYPE_UINT8);
    uint8_t val = 0;
    int result = 1;

    if (var->type == BARANIUM_VARIABLE_TYPE_OBJECT)
        val = (uint8_t)var->value.num64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_BOOL)
        val = (uint8_t)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_FLOAT)
        val = (uint8_t)var->value.numfloat;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT32)
        val = (uint8_t)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT32)
        val = (uint8_t)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_STRING && string_is_number(var->value.str))
        val = (uint8_t)string_to_number(var->value.str);
    else if (var->type == BARANIUM_VARIABLE_TYPE_DOUBLE)
        val = (uint8_t)var->value.numdouble;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT8)
        val = (uint8_t)var->value.snum8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT16)
        val = (uint8_t)var->value.snum16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT16)
        val = (uint8_t)var->value.num16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT64)
        val = (uint8_t)var->value.snum64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT64)
        val = (uint8_t)var->value.num64;
    else result = 0;

    var->value.num8 = val;
    var->size = size;
    return result;
}

int baranium_compiled_variable_as_int16(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(BARANIUM_VARIABLE_TYPE_INT16);
    int16_t val = 0;
    int result = 1;

    if (var->type == BARANIUM_VARIABLE_TYPE_OBJECT)
        val = (int16_t)var->value.num64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_BOOL)
        val = (int16_t)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_FLOAT)
        val = (int16_t)var->value.numfloat;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT32)
        val = (int16_t)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT32)
        val = (int16_t)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_STRING && string_is_number(var->value.str))
        val = (int16_t)string_to_number(var->value.str);
    else if (var->type == BARANIUM_VARIABLE_TYPE_DOUBLE)
        val = (int16_t)var->value.numdouble;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT8)
        val = (int16_t)var->value.snum8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT8)
        val = (int16_t)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT16)
        val = (int16_t)var->value.num16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT64)
        val = (int16_t)var->value.snum64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT64)
        val = (int16_t)var->value.num64;
    else result = 0;

    var->value.snum16 = val;
    var->size = size;
    return result;
}

int baranium_compiled_variable_as_uint16(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(BARANIUM_VARIABLE_TYPE_UINT16);
    uint16_t val = 0;
    int result = 1;

    if (var->type == BARANIUM_VARIABLE_TYPE_OBJECT)
        val = (uint16_t)var->value.num64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_BOOL)
        val = (uint16_t)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_FLOAT)
        val = (uint16_t)var->value.numfloat;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT32)
        val = (uint16_t)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT32)
        val = (uint16_t)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_STRING && string_is_number(var->value.str))
        val = (uint16_t)string_to_number(var->value.str);
    else if (var->type == BARANIUM_VARIABLE_TYPE_DOUBLE)
        val = (uint16_t)var->value.numdouble;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT8)
        val = (uint16_t)var->value.snum8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT8)
        val = (uint16_t)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT16)
        val = (uint16_t)var->value.snum16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT64)
        val = (uint16_t)var->value.snum64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT64)
        val = (uint16_t)var->value.num64;
    else result = 0;

    var->value.num16 = val;
    var->size = size;
    return result;
}

int baranium_compiled_variable_as_int64(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(BARANIUM_VARIABLE_TYPE_INT64);
    int64_t val = 0;
    int result = 1;

    if (var->type == BARANIUM_VARIABLE_TYPE_OBJECT)
        val = (int64_t)var->value.num64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_BOOL)
        val = (int64_t)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_FLOAT)
        val = (int64_t)var->value.numfloat;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT32)
        val = (int64_t)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT32)
        val = (int64_t)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_STRING && string_is_number(var->value.str))
        val = (int64_t)string_to_number(var->value.str);
    else if (var->type == BARANIUM_VARIABLE_TYPE_DOUBLE)
        val = (int64_t)var->value.numdouble;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT8)
        val = (int64_t)var->value.snum8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT8)
        val = (int64_t)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT16)
        val = (int64_t)var->value.snum16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT16)
        val = (int64_t)var->value.num16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT64)
        val = (int64_t)var->value.num64;
    else result = 0;

    var->value.snum64 = val;
    var->size = size;
    return result;
}

int baranium_compiled_variable_as_uint64(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(BARANIUM_VARIABLE_TYPE_UINT64);
    uint64_t val = 0;
    int result = 1;

    if (var->type == BARANIUM_VARIABLE_TYPE_OBJECT)
        val = (uint64_t)var->value.num64;
    else if (var->type == BARANIUM_VARIABLE_TYPE_BOOL)
        val = (uint64_t)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_FLOAT)
        val = (uint64_t)var->value.numfloat;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT32)
        val = (uint64_t)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT32)
        val = (uint64_t)var->value.num32;
    else if (var->type == BARANIUM_VARIABLE_TYPE_STRING && string_is_number(var->value.str))
        val = (uint64_t)string_to_number(var->value.str);
    else if (var->type == BARANIUM_VARIABLE_TYPE_DOUBLE)
        val = (uint64_t)var->value.numdouble;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT8)
        val = (uint64_t)var->value.snum8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT8)
        val = (uint64_t)var->value.num8;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT16)
        val = (uint64_t)var->value.snum16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_UINT16)
        val = (uint64_t)var->value.num16;
    else if (var->type == BARANIUM_VARIABLE_TYPE_INT64)
        val = (uint64_t)var->value.snum64;
    else result = 0;

    var->value.num64 = val;
    var->size = size;
    return result;
}

void baranium_compiled_variable_convert_to_type(baranium_compiled_variable* var, baranium_variable_type_t targetType)
{
    if (targetType == BARANIUM_VARIABLE_TYPE_INVALID || targetType == BARANIUM_VARIABLE_TYPE_VOID)
        return;
    
    if (targetType == var->type)
        return;

    uint8_t status = 0;

    baranium_variable_type_t oldType = var->type;
    baranium_value_t oldValue = var->value;

    if (targetType == BARANIUM_VARIABLE_TYPE_OBJECT)
        status = baranium_compiled_variable_as_object(var);
    else if (targetType == BARANIUM_VARIABLE_TYPE_FLOAT)
        status = baranium_compiled_variable_as_float(var);
    else if (targetType == BARANIUM_VARIABLE_TYPE_BOOL)
        status = baranium_compiled_variable_as_bool(var);
    else if (targetType == BARANIUM_VARIABLE_TYPE_INT32)
        status = baranium_compiled_variable_as_int(var);
    else if (targetType == BARANIUM_VARIABLE_TYPE_UINT32)
        status = baranium_compiled_variable_as_uint(var);
    else if (targetType == BARANIUM_VARIABLE_TYPE_DOUBLE)
        status = baranium_compiled_variable_as_double(var);
    else if (targetType == BARANIUM_VARIABLE_TYPE_INT8)
        status = baranium_compiled_variable_as_byte(var);
    else if (targetType == BARANIUM_VARIABLE_TYPE_UINT8)
        status = baranium_compiled_variable_as_ubyte(var);
    else if (targetType == BARANIUM_VARIABLE_TYPE_INT16)
        status = baranium_compiled_variable_as_int16(var);
    else if (targetType == BARANIUM_VARIABLE_TYPE_UINT16)
        status = baranium_compiled_variable_as_uint16(var);
    else if (targetType == BARANIUM_VARIABLE_TYPE_INT64)
        status = baranium_compiled_variable_as_int64(var);
    else if (targetType == BARANIUM_VARIABLE_TYPE_UINT64)
        status = baranium_compiled_variable_as_uint64(var);
    else if (targetType == BARANIUM_VARIABLE_TYPE_STRING)
    {
        char* stringifiedVersion = baranium_variable_stringify(var->type, var->value);
        size_t stringSize = strlen(stringifiedVersion);
        if (!stringifiedVersion)
            return;
        if (var->type == BARANIUM_VARIABLE_TYPE_STRING)
            var->value.ptr = realloc(var->value.ptr, stringSize+1);
        else
            var->value.ptr = malloc(stringSize+1);
        memcpy(var->value.ptr, stringifiedVersion, stringSize);
        ((uint8_t*)var->value.ptr)[stringSize] = 0;
        var->size = strlen(var->value.ptr);
        var->type = targetType;
        return;
    }

    if (oldType == BARANIUM_VARIABLE_TYPE_STRING && oldValue.ptr != NULL)
        free(oldValue.ptr);

    if (status)
        var->type = targetType;
}

void baranium_compiled_variable_combine(baranium_compiled_variable* lhs, baranium_compiled_variable* rhs, uint8_t operation, baranium_variable_type_t type)
{
    if (lhs == NULL || rhs == NULL || operation == BARANIUM_VARIABLE_OPERATION_NONE)
        return;

    baranium_variable_type_t resultType = type;
    if (type == BARANIUM_VARIABLE_TYPE_INVALID || type == BARANIUM_VARIABLE_TYPE_VOID)
        resultType = lhs->type;
    
    if (lhs->type == BARANIUM_VARIABLE_TYPE_STRING || rhs->type == BARANIUM_VARIABLE_TYPE_STRING)
        resultType = BARANIUM_VARIABLE_TYPE_STRING;
    
    if (resultType == BARANIUM_VARIABLE_TYPE_INVALID || resultType == BARANIUM_VARIABLE_TYPE_VOID)
        return;
    
    if (operation != BARANIUM_VARIABLE_OPERATION_ADD && resultType == BARANIUM_VARIABLE_TYPE_STRING) // also special case
    {
        LOGERROR("Strings can only be combined using '+'");
        return;
    }

    if (operation == BARANIUM_VARIABLE_OPERATION_ADD && resultType == BARANIUM_VARIABLE_TYPE_STRING) // very special case
    {
        baranium_compiled_variable* string = lhs;
        baranium_compiled_variable* non_string = rhs;
        if (rhs->type == BARANIUM_VARIABLE_TYPE_STRING)
        {
            string = rhs;
            non_string = lhs;
        }

        char* string1 = baranium_variable_stringify(non_string->type, non_string->value);
        size_t size = strlen(string->value.str) + strlen(string1) + 1;
        string->value.ptr = realloc(string->value.ptr, size);
        memcpy(string->value.str + strlen(string->value.str), string1, strlen(string1));
        string->value.str[size-1] = 0;
        string->type = BARANIUM_VARIABLE_TYPE_STRING;
        string->size = size;

        // lhs is always the resulting output
        if (string == rhs)
        {
            baranium_compiled_variable tmp = *rhs;
            *rhs = *lhs;
            *lhs = tmp;
        }
        return;
    }

    if (rhs->type != lhs->type)
        baranium_compiled_variable_convert_to_type(rhs, lhs->type);

    baranium_compiled_variable_perform_arithmetic_operation(&lhs->value, &rhs->value, lhs->type, operation);

    if (lhs->type != resultType)
        baranium_compiled_variable_convert_to_type(lhs, resultType);
}
