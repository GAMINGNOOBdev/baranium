#include <baranium/backend/varmath.h>
#include <baranium/backend/errors.h>
#include <baranium/cpu/bstack.h>
#include <baranium/variable.h>
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
    result->type = (baranium_variable_type_t)bstack_pop(cpu->stack);
    size_t size = result->size = (size_t)bstack_pop(cpu->stack);
    void* value = malloc(result->size);
    result->value = value;
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

    if (result->type == VARIABLE_TYPE_STRING)
    {
        result->size++;
        value = malloc(result->size);
        memset(value, 0, result->size);
        memcpy(value, result->value, result->size-1);
        free(result->value);
        result->value = value;
    }

    return result;
}

void baranium_compiled_variable_push_to_stack(bcpu* cpu, baranium_compiled_variable* var)
{
    baranium_variable_type_t type = var->type;
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
        if(operation == BARANIUM_VARIABLE_OPERATION_MOD) { if (exprr == 0 && baranium_get_context() != NULL) \
                                                         {\
                                                            baranium_get_context()->cpu->killTriggered = 1;\
                                                            baranium_get_context()->cpu->flags.FORCED_KILL = 1;\
                                                            bstack_push(baranium_get_context()->cpu->stack, ERR_DIV_BY_ZERO);\
                                                         } \
                                                         else PerformOperation(out, exprl, exprr, %); } \
        if(operation == BARANIUM_VARIABLE_OPERATION_DIV) { if (exprr == 0 && baranium_get_context() != NULL) \
                                                         {\
                                                            baranium_get_context()->cpu->killTriggered = 1;\
                                                            baranium_get_context()->cpu->flags.FORCED_KILL = 1;\
                                                            bstack_push(baranium_get_context()->cpu->stack, ERR_DIV_BY_ZERO);\
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

void baranium_compiled_variable_perform_arithmetic_operation(void* dataLeft, void* dataRight, baranium_variable_type_t target, uint8_t operation)
{
    if (target == VARIABLE_TYPE_INVALID || target == VARIABLE_TYPE_VOID || target == VARIABLE_TYPE_STRING)
        return;

    if (target == VARIABLE_TYPE_OBJECT)
    {
        ForeachOperation(operation, *((int64_t*)dataLeft), *((int64_t*)dataLeft), *((int64_t*)dataRight));
        return;
    }

    if (target == VARIABLE_TYPE_FLOAT)
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

    if (target == VARIABLE_TYPE_BOOL)
    {
        ForeachOperation(operation, *((uint8_t*)dataLeft), *((uint8_t*)dataLeft), *((uint8_t*)dataRight));
        return;
    }

    if (target == VARIABLE_TYPE_INT)
    {
        ForeachOperation(operation, *((int32_t*)dataLeft), *((int32_t*)dataLeft), *((int32_t*)dataRight));
        return;
    }

    if (target == VARIABLE_TYPE_UINT)
    {
        ForeachOperation(operation, *((uint32_t*)dataLeft), *((uint32_t*)dataLeft), *((uint32_t*)dataRight));
        return;
    }
}

static const char* string_ptr_tmp = NULL;
static uint64_t string_to_num_tmp = 0;
static double string_to_num_float_tmp = 0.0;
static uint8_t string_to_num_is_float = 0;

uint8_t string_is_number(const char* str)
{
    string_to_num_float_tmp = 0;
    string_to_num_is_float = 0;
    string_to_num_tmp = 0;
    string_ptr_tmp = str;

    size_t len = strlen(str);
    size_t index = 0;
    char c = str[index];
    uint16_t decimalPlaces = 1;

    if (c == '+' || c == '-')
        index++;

    // check for hex/binary formats beforehand
    if (len > 2)
    {
        c = str[1];
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
    string_to_num_float_tmp = string_to_num_tmp;

    return 1;
}

uint64_t string_to_number(const char* str)
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

int baranium_compiled_variable_as_object(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(VARIABLE_TYPE_OBJECT);
    int64_t val = 0;
    int result = 1;

    if (var->type == VARIABLE_TYPE_FLOAT)
        val = (int64_t)*((float*)var->value);
    else if (var->type == VARIABLE_TYPE_BOOL)
        val = (int64_t)*((uint8_t*)var->value);
    else if (var->type == VARIABLE_TYPE_INT)
        val = (int64_t)*((int32_t*)var->value);
    else if (var->type == VARIABLE_TYPE_UINT)
        val = (int64_t)*((uint32_t*)var->value);
    else if (var->type == VARIABLE_TYPE_STRING && string_is_number((const char*)var->value))
        val = (int64_t)string_to_number((const char*)var->value);
    else result = 0;

    var->value = realloc(var->value, size);
    *((int64_t*)var->value) = val;
    var->size = size;
    return result;
}

int baranium_compiled_variable_as_float(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(VARIABLE_TYPE_FLOAT);
    float val = 0;
    int result = 1;

    if (var->type == VARIABLE_TYPE_OBJECT)
        val = (float)*((int64_t*)var->value);
    else if (var->type == VARIABLE_TYPE_BOOL)
        val = (float)*((uint8_t*)var->value);
    else if (var->type == VARIABLE_TYPE_INT)
        val = (float)*((int32_t*)var->value);
    else if (var->type == VARIABLE_TYPE_UINT)
        val = (float)*((uint32_t*)var->value);
    else if (var->type == VARIABLE_TYPE_STRING && string_is_number((const char*)var->value))
        val = (float)string_to_float((const char*)var->value);
    else result = 0;

    var->value = realloc(var->value, size);
    *((float*)var->value) = val;
    var->size = size;
    return result;
}

int baranium_compiled_variable_as_bool(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(VARIABLE_TYPE_BOOL);
    uint8_t val = 0;
    int result = 1;

    if (var->type == VARIABLE_TYPE_OBJECT)
        val = 0 < *((int64_t*)var->value);
    else if (var->type == VARIABLE_TYPE_FLOAT)
        val = 0 < *((float*)var->value);
    else if (var->type == VARIABLE_TYPE_INT)
        val = 0 < *((int32_t*)var->value);
    else if (var->type == VARIABLE_TYPE_UINT)
        val = 0 < *((uint32_t*)var->value);
    else if (var->type == VARIABLE_TYPE_STRING && string_is_number((const char*)var->value))
        val = 0 < string_to_number((const char*)var->value);
    else result = 0;

    var->value = realloc(var->value, size);
    *((uint8_t*)var->value) = val;
    var->size = size;
    return result;
}

int baranium_compiled_variable_as_int(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(VARIABLE_TYPE_INT);
    int32_t val = 0;
    int result = 1;

    if (var->type == VARIABLE_TYPE_OBJECT)
        val = (int32_t)*((int64_t*)var->value);
    else if (var->type == VARIABLE_TYPE_BOOL)
        val = (int32_t)*((uint8_t*)var->value);
    else if (var->type == VARIABLE_TYPE_FLOAT)
        val = (int32_t)*((float*)var->value);
    else if (var->type == VARIABLE_TYPE_UINT)
        val = (int32_t)*((uint32_t*)var->value);
    else if (var->type == VARIABLE_TYPE_STRING && string_is_number((const char*)var->value))
        val = (int32_t)string_to_number((const char*)var->value);
    else result = 0;

    var->value = realloc(var->value, size);
    *((int32_t*)var->value) = val;
    var->size = size;
    return result;
}

int baranium_compiled_variable_as_uint(baranium_compiled_variable* var)
{
    size_t size = baranium_variable_get_size_of_type(VARIABLE_TYPE_UINT);
    uint32_t val = 0;
    int result = 1;

    if (var->type == VARIABLE_TYPE_OBJECT)
        val = (uint32_t)*((int64_t*)var->value);
    else if (var->type == VARIABLE_TYPE_BOOL)
        val = (uint32_t)*((uint8_t*)var->value);
    else if (var->type == VARIABLE_TYPE_FLOAT)
        val = (uint32_t)*((float*)var->value);    
    else if (var->type == VARIABLE_TYPE_INT)
        val = (uint32_t)*((int32_t*)var->value);
    else if (var->type == VARIABLE_TYPE_STRING && string_is_number((const char*)var->value))
        val = (uint32_t)string_to_number((const char*)var->value);
    else result = 0;

    var->value = realloc(var->value, size);
    *((uint32_t*)var->value) = val;
    var->size = size;
    return result;
}

void baranium_compiled_variable_convert_to_type(baranium_compiled_variable* var, baranium_variable_type_t targetType)
{
    if (targetType == VARIABLE_TYPE_INVALID || targetType == VARIABLE_TYPE_VOID)
        return;
    
    if (targetType == var->type)
        return;

    uint8_t status = 0;

    if (targetType == VARIABLE_TYPE_OBJECT)
        status = baranium_compiled_variable_as_object(var);
    else if (targetType == VARIABLE_TYPE_FLOAT)
        status = baranium_compiled_variable_as_float(var);
    else if (targetType == VARIABLE_TYPE_BOOL)
        status = baranium_compiled_variable_as_bool(var);
    else if (targetType == VARIABLE_TYPE_INT)
        status = baranium_compiled_variable_as_int(var);
    else if (targetType == VARIABLE_TYPE_UINT)
        status = baranium_compiled_variable_as_uint(var);
    else if (targetType == VARIABLE_TYPE_STRING)
    {
        char* stringifiedVersion = baranium_variable_stringify(var->type, var->value);
        size_t stringSize = strlen(stringifiedVersion);
        if (!stringifiedVersion)
            return;
        var->value = realloc(var->value, stringSize+1);
        memcpy(var->value, stringifiedVersion, stringSize);
        ((uint8_t*)var->value)[stringSize] = 0;
        var->size = strlen(var->value);
        var->type = targetType;
        return;
    }

    if (status)
        var->type = targetType;
}

void baranium_compiled_variable_combine(baranium_compiled_variable* lhs, baranium_compiled_variable* rhs, uint8_t operation, baranium_variable_type_t type)
{
    if (lhs == NULL || rhs == NULL || operation == BARANIUM_VARIABLE_OPERATION_NONE)
        return;

    baranium_variable_type_t resultType = type;
    if (type == VARIABLE_TYPE_INVALID || type == VARIABLE_TYPE_VOID)
        resultType = lhs->type;
    
    if (resultType == VARIABLE_TYPE_INVALID || resultType == VARIABLE_TYPE_VOID)
        return;
    
    if (operation != BARANIUM_VARIABLE_OPERATION_ADD && resultType == VARIABLE_TYPE_STRING) // also special case
    {
        LOGERROR("Strings can only be combined using '+'");
        return;
    }

    if (operation == BARANIUM_VARIABLE_OPERATION_ADD && resultType == VARIABLE_TYPE_STRING) // very special case
    {
        void* value = (void*)stringf("%s%s", baranium_variable_stringify(lhs->type, lhs->value), baranium_variable_stringify(rhs->type, rhs->value));
        free(lhs->value);
        size_t size = strlen(value)+1;
        lhs->value = malloc(size);
        memset(lhs->value, 0, size);
        memcpy(lhs->value, value, size-1);
        lhs->type = VARIABLE_TYPE_STRING;
        return;
    }

    if (rhs->type != lhs->type)
        baranium_compiled_variable_convert_to_type(rhs, lhs->type);

    baranium_compiled_variable_perform_arithmetic_operation(lhs->value, rhs->value, lhs->type, operation);

    if (lhs->type != resultType)
        baranium_compiled_variable_convert_to_type(lhs, resultType);
}
