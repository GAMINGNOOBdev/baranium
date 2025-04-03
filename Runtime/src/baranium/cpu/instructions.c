#include <baranium/backend/bfuncmgr.h>
#include <baranium/backend/bvarmgr.h>
#include <baranium/backend/varmath.h>
#include <baranium/backend/errors.h>
#include <baranium/variable.h>
#include <baranium/callback.h>
#include <baranium/runtime.h>
#include <baranium/logging.h>
#include <baranium/defines.h>
#include <baranium/bcpu.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>

#define CMP_LESS_THAN     0x18
#define CMP_LESS_EQUAL    0x38
#define CMP_GREATER_THAN  0x28
#define CMP_GREATER_EQUAL 0x48
#define CMP_EQUAL         0x08
#define CMP_NOTEQUAL      0xF8

#define CMP_AND 0
#define CMP_OR  1

void INVALID_OPCODE(bcpu* cpu)
{
    if (!cpu) return;

    cpu->kill_triggered = true;
    LOGERROR("invalid opcode, quitting...");
}

void NOP(bcpu* cpu) {if(!cpu) return;}

void CCF(bcpu* cpu)
{
    if (!cpu) return;

    cpu->flags.CMP = false;
}

void SCF(bcpu* cpu)
{
    if (!cpu) return;

    cpu->flags.CMP = true;
}

void CCV(bcpu* cpu)
{
    if (!cpu) return;

    cpu->cv = 0;
}

void ICV(bcpu* cpu)
{
    if (!cpu) return;

    if (cpu->cv)
        cpu->cv = 0;
    else
        cpu->cv = 1;
}

void PUSHCV(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable var;
    var.size = sizeof(uint8_t);
    var.type = BARANIUM_VARIABLE_TYPE_BOOL;
    var.value.num8 = cpu->cv;
    baranium_compiled_variable_push_to_stack(cpu, &var);
}

void POPCV(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable var = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &var);
    cpu->cv = var.value.num8;
}

void PUSHVAR(bcpu* cpu)
{
    if (!cpu) return;

    index_t id = (index_t)cpu->fetch(cpu, 64);
    bvarmgr* varmgr = cpu->runtime->varmgr;
    bvarmgr_n* var = bvarmgr_get(varmgr, id);
    if (var == NULL)
    {
        LOGERROR(stringf("Variable/Field with ID '%d' not found", id));
        bstack_push(cpu->stack, BARANIUM_ERROR_VAR_NOT_FOUND);
        cpu->flags.FORCED_KILL = true;
        cpu->kill_triggered = true;
        return;
    }

    size_t size = 0;
    baranium_value_t value = {0};
    baranium_variable_type_t type = BARANIUM_VARIABLE_TYPE_INVALID;
    if (var->isVariable)
    {
        size = var->variable->size;
        type = var->variable->type;
        value = var->variable->value;
    }
    else
    {
        size = var->field->size;
        type = var->field->type;
        value = var->field->value;
    }

    if (type == BARANIUM_VARIABLE_TYPE_VOID || type == BARANIUM_VARIABLE_TYPE_INVALID)
    {
        LOGERROR(stringf("Variable/Field with ID '%d' cannot be pushed: Invalid type", id));
        bstack_push(cpu->stack, BARANIUM_ERROR_VAR_INVALID_TYPE);
        cpu->flags.FORCED_KILL = true;
        cpu->kill_triggered = true;
        return;
    }

    uint64_t data = 0;
    if (size <= 8)
    {
        memcpy(&data, &value.num64, size);
        bstack_push(cpu->stack, data);
    }
    else
    {
        void* basevalue = &value.num64;
        if (type == BARANIUM_VARIABLE_TYPE_STRING)
            basevalue = value.ptr;

        size_t leftOverSize = size;
        size_t index = 0;
        void* valPtr;
        for (index = 0; index * 8 < size; index++)
        {
            if (leftOverSize < 8)
                break;
            valPtr = (void*)((uint64_t)basevalue + index*8);
            memcpy(&data, valPtr, 8);
            leftOverSize -= 8;
            bstack_push(cpu->stack, data);
        }
        valPtr = (void*)((uint64_t)basevalue + index*8);
        data = 0;
        memcpy(&data, valPtr, leftOverSize);
        bstack_push(cpu->stack, data);
    }

    bstack_push(cpu->stack, (uint64_t)size);
    bstack_push(cpu->stack, (uint64_t)type);
}

void POPVAR(bcpu* cpu)
{
    if (!cpu) return;

    index_t id = (index_t)cpu->fetch(cpu, 64);
    bvarmgr* varmgr = cpu->runtime->varmgr;
    bvarmgr_n* var = bvarmgr_get(varmgr, id);
    if (var == NULL)
    {
        LOGERROR(stringf("Variable/Field with ID '%d' not found", id));
        bstack_push(cpu->stack, BARANIUM_ERROR_VAR_NOT_FOUND);
        cpu->flags.FORCED_KILL = true;
        cpu->kill_triggered = true;
        return;
    }

    baranium_value_t value = {0};
    baranium_variable_type_t type = BARANIUM_VARIABLE_TYPE_INVALID;
    size_t size = 0;
    if (var->isVariable)
    {
        type = var->variable->type;
        value = var->variable->value;
        size = var->variable->size;
    }
    else
    {
        type = var->field->type;
        value = var->field->value;
        size = var->field->size;
    }

    if (type == BARANIUM_VARIABLE_TYPE_VOID || type == BARANIUM_VARIABLE_TYPE_INVALID)
    {
        LOGERROR(stringf("Variable/Field with ID '%d' cannot be assigned: Invalid type", id));
        bstack_push(cpu->stack, BARANIUM_ERROR_VAR_INVALID_TYPE);
        cpu->flags.FORCED_KILL = true;
        cpu->kill_triggered = true;
        return;
    }

    baranium_compiled_variable newvar = {.type=type, .value=value, .size=size};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &newvar);
    if (!baranium_variable_type_size_interchangable(type, newvar.type))
    {
        LOGERROR(stringf("Variable/Field with ID '%d' cannot be assigned: Non-matching types of variable and assign value", id));
        bstack_push(cpu->stack, BARANIUM_ERROR_VAR_INVALID_TYPE);
        if (newvar.type == BARANIUM_VARIABLE_TYPE_STRING)
            free(newvar.value.ptr);
        cpu->flags.FORCED_KILL = true;
        cpu->kill_triggered = true;
        return;
    }
    baranium_compiled_variable_convert_to_type(&newvar, type);

    // assign new size and value pointers for the variable/field
    if (var->isVariable)
    {
        var->variable->size = newvar.size;
        var->variable->value = newvar.value;
    }
    else
    {
        var->field->size = newvar.size;
        var->field->value = newvar.value;
    }
}

void PUSH(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t value = cpu->fetch(cpu, 64);
    bstack_push(cpu->stack, value);
}

void CALL(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t id = cpu->fetch(cpu, 64);
    bstack_push(cpu->ip_stack, cpu->ip);

    LOGDEBUG(stringf("calling function with id '%lld' (current IP: %lld)", id, cpu->ip));

    baranium_runtime* runtime = baranium_get_context();
    baranium_callback_list_entry* callback = baranium_callback_find_by_id( id);
    LOGDEBUG(stringf("callback id: %lld callback ptr: 0x%16.16x", id, (uint64_t)callback));
    baranium_function* func = baranium_function_manager_get(runtime->function_manager, id);

    if (callback != NULL)
    {
        baranium_callback_data_list_t data = {
            .dataptr = NULL,
            .datatypes = NULL,
            .num_data = callback->numParams,
        };
        if (data.num_data > 0 && data.num_data != -1)
        {
            data.dataptr = malloc(sizeof(baranium_value_t)*data.num_data);
            data.datatypes = malloc(sizeof(baranium_variable_type_t));

            baranium_compiled_variable tmp;
            for (int i = 0; i < data.num_data; i++)
            {
                tmp = (baranium_compiled_variable){.type=0,.value={0},.size=0};
                baranium_compiled_variable_pop_from_stack_into_variable(cpu, &tmp);
                data.dataptr[i] = tmp.value;
                data.datatypes[i] = tmp.type;
            }
        }
        LOGDEBUG(stringf("callback call: data.dataptr{0x%16.16x} data.datatypes{0x%16.16x} data.numData=%d", (uint64_t)data.dataptr, (uint64_t)data.datatypes, data.num_data));
        callback->callback(&data);
        if (data.num_data > 0 && data.num_data != -1)
        {
            for (int i = 0; i < data.num_data; i++)
                if (data.datatypes[i] == BARANIUM_VARIABLE_TYPE_STRING)
                    free(data.dataptr[i].ptr);
            free(data.dataptr);
            free(data.datatypes);
        }
    }
    else
        baranium_function_call(func, NULL, NULL, -1);

    baranium_function_dispose(func);

    cpu->ip = bstack_pop(cpu->ip_stack);
    LOGDEBUG(stringf("finished calling function with id '%lld' (current IP: %lld)", id, cpu->ip));
}

void RET(bcpu* cpu)
{
    if (!cpu) return;

    cpu->kill_triggered = 1;
}

void JMP(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t addr = cpu->fetch(cpu, 64);
    cpu->ip = addr;
}

void JMPOFF(bcpu* cpu)
{
    if (!cpu) return;

    int16_t offset = cpu->fetch(cpu, 16);
    cpu->ip += offset;
}

void JMPC(bcpu* cpu)
{
    if (!cpu) return;
    
    uint64_t addr = cpu->fetch(cpu, 64);
    if (!cpu->flags.CMP) return;
    if (cpu->cv == 0)
        return;

    cpu->ip = addr;
}

void JMPCOFF(bcpu* cpu)
{
    if (!cpu) return;
    
    uint16_t offset = cpu->fetch(cpu, 16);
    if (!cpu->flags.CMP) return;
    if (cpu->cv == 0)
        return;

    cpu->ip += offset;
}

void MOD(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable divisor = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &divisor);
    baranium_compiled_variable divident = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &divident);

    baranium_compiled_variable_combine(&divisor, &divident, BARANIUM_VARIABLE_OPERATION_MOD, divisor.type);

    baranium_compiled_variable_push_to_stack(cpu, &divisor);
    if (divident.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(divident.value.ptr);
    if (divisor.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(divisor.value.ptr);
}

void DIV(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable divisor = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &divisor);
    baranium_compiled_variable divident = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &divident);

    baranium_compiled_variable_combine(&divisor, &divident, BARANIUM_VARIABLE_OPERATION_DIV, divisor.type);

    baranium_compiled_variable_push_to_stack(cpu, &divisor);
    if (divident.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(divident.value.ptr);
    if (divisor.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(divisor.value.ptr);
}

void MUL(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable val0 = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val0);
    baranium_compiled_variable val1 = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val1);

    baranium_compiled_variable_combine(&val0, &val1, BARANIUM_VARIABLE_OPERATION_MUL, val0.type);

    baranium_compiled_variable_push_to_stack(cpu, &val0);
    if (val1.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val1.value.ptr);
    if (val0.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val0.value.ptr);
}

void SUB(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable val0 = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val0);
    baranium_compiled_variable val1 = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val1);

    baranium_compiled_variable_combine(&val0, &val1, BARANIUM_VARIABLE_OPERATION_SUB, val0.type);

    baranium_compiled_variable_push_to_stack(cpu, &val0);
    if (val1.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val1.value.ptr);
    if (val0.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val0.value.ptr);
}

void ADD(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable val0 = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val0);
    baranium_compiled_variable val1 = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val1);

    baranium_compiled_variable_combine(&val0, &val1, BARANIUM_VARIABLE_OPERATION_ADD, val0.type);

    baranium_compiled_variable_push_to_stack(cpu, &val0);
    if (val1.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val1.value.ptr);
    if (val0.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val0.value.ptr);
}

void AND(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable val0 = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val0);
    baranium_compiled_variable val1 = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val1);

    baranium_compiled_variable_combine(&val0, &val1, BARANIUM_VARIABLE_OPERATION_AND, val0.type);

    baranium_compiled_variable_push_to_stack(cpu, &val0);
    if (val1.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val1.value.ptr);
    if (val0.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val0.value.ptr);
}

void OR(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable val0 = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val0);
    baranium_compiled_variable val1 = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val1);

    baranium_compiled_variable_combine(&val0, &val1, BARANIUM_VARIABLE_OPERATION_OR, val0.type);

    baranium_compiled_variable_push_to_stack(cpu, &val0);
    if (val1.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val1.value.ptr);
    if (val0.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val0.value.ptr);
}

void XOR(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable val0 = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val0);
    baranium_compiled_variable val1 = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val1);

    baranium_compiled_variable_combine(&val0, &val1, BARANIUM_VARIABLE_OPERATION_XOR, val0.type);

    baranium_compiled_variable_push_to_stack(cpu, &val0);
    if (val1.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val1.value.ptr);
    if (val0.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val0.value.ptr);
}

void SHFTL(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable val0 = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val0);
    baranium_compiled_variable val1 = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val1);

    baranium_compiled_variable_combine(&val0, &val1, BARANIUM_VARIABLE_OPERATION_SHFTL, val0.type);

    baranium_compiled_variable_push_to_stack(cpu, &val0);
    if (val1.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val1.value.ptr);
    if (val0.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val0.value.ptr);
}

void SHFTR(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable val0 = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val0);
    baranium_compiled_variable val1 = {0,{0},0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val1);

    baranium_compiled_variable_combine(&val0, &val1, BARANIUM_VARIABLE_OPERATION_SHFTR, val0.type);

    baranium_compiled_variable_push_to_stack(cpu, &val0);
    if (val1.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val1.value.ptr);
    if (val0.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val0.value.ptr);
}

void CMP(bcpu* cpu)
{
    if (!cpu) return;
    if (!cpu->flags.CMP) return;

    uint8_t operation = cpu->fetch(cpu, 8);

    baranium_compiled_variable val1 = {0,{0}, 0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val1);
    baranium_compiled_variable val0 = {0,{0}, 0};
    baranium_compiled_variable_pop_from_stack_into_variable(cpu, &val0);

    // yes this is a somewhat lazy way but hey, it's somewhat logical as well so shut up
    if (val1.type == BARANIUM_VARIABLE_TYPE_STRING || val0.type == BARANIUM_VARIABLE_TYPE_STRING)
    {
        baranium_compiled_variable_convert_to_type(&val0, BARANIUM_VARIABLE_TYPE_STRING);
        char* value0 = (char*)val0.value.ptr;
        baranium_compiled_variable_convert_to_type(&val1, BARANIUM_VARIABLE_TYPE_STRING);
        char* value1 = (char*)val1.value.ptr;

        if (operation != CMP_EQUAL || operation == CMP_NOTEQUAL)
        {
            LOGWARNING(stringf("comparing '%s' and '%s' for something that is not == or !=, WTF ARE YOU DOING, ima just default to result 0 just for the operation to still succeed but i highly suggest checking your code please" ));
        }

        if (operation == CMP_EQUAL)
        {
            cpu->cv = (strcmp(value0, value1) == 0) ? 1 : 0;
            LOGDEBUG(stringf("comparing '%s' and '%s' for CMP_EQUAL", value0, value1));
        }
        if (operation == CMP_NOTEQUAL)
        {
            cpu->cv = (strcmp(value0, value1) == 0) ? 1 : 0;
            LOGDEBUG(stringf("comparing '%s' and '%s' for CMP_NOTEQUAL", value0, value1));
        }
    }
    else if (val1.type == BARANIUM_VARIABLE_TYPE_FLOAT || val0.type == BARANIUM_VARIABLE_TYPE_FLOAT)
    {
        baranium_compiled_variable_convert_to_type(&val0, BARANIUM_VARIABLE_TYPE_FLOAT);
        float value0 = val0.value.numfloat;
        baranium_compiled_variable_convert_to_type(&val1, BARANIUM_VARIABLE_TYPE_FLOAT);
        float value1 = val1.value.numfloat;

        if (operation == CMP_EQUAL)
        {
            cpu->cv = (value0 == value1) ? 1 : 0;
            LOGDEBUG(stringf("comparing %f and %f for CMP_EQUAL", value0, value1));
        }
        if (operation == CMP_NOTEQUAL)
        {
            cpu->cv = (value0 != value1) ? 1 : 0;
            LOGDEBUG(stringf("comparing %f and %f for CMP_NOTEQUAL", value0, value1));
        }
        if (operation == CMP_LESS_THAN)
        {
            cpu->cv = (value0 < value1) ? 1 : 0;
            LOGDEBUG(stringf("comparing %f and %f for CMP_LESS_THAN", value0, value1));
        }
        if (operation == CMP_LESS_EQUAL)
        {
            cpu->cv = (value0 <= value1) ? 1 : 0;
            LOGDEBUG(stringf("comparing %f and %f for CMP_LESS_EQUAL", value0, value1));
        }
        if (operation == CMP_GREATER_THAN)
        {
            cpu->cv = (value0 > value1) ? 1 : 0;
            LOGDEBUG(stringf("comparing %f and %f for CMP_GREATER_THAN", value0, value1));
        }
        if (operation == CMP_GREATER_EQUAL)
        {
            cpu->cv = (value0 >= value1) ? 1 : 0;
            LOGDEBUG(stringf("comparing %f and %f for CMP_GREATER_EQUAL", value0, value1));
        }
    }
    else
    {
        baranium_compiled_variable_convert_to_type(&val0, BARANIUM_VARIABLE_TYPE_OBJECT);
        int64_t value0 = val0.value.num64;
        baranium_compiled_variable_convert_to_type(&val1, BARANIUM_VARIABLE_TYPE_OBJECT);
        int64_t value1 = val1.value.num64;

        if (operation == CMP_EQUAL)
        {
            cpu->cv = (value0 == value1) ? 1 : 0;
            LOGDEBUG(stringf("comparing %lld and %lld for CMP_EQUAL", value0, value1));
        }
        if (operation == CMP_NOTEQUAL)
        {
            cpu->cv = (value0 != value1) ? 1 : 0;
            LOGDEBUG(stringf("comparing %lld and %lld for CMP_NOTEQUAL", value0, value1));
        }
        if (operation == CMP_LESS_THAN)
        {
            cpu->cv = (value0 < value1) ? 1 : 0;
            LOGDEBUG(stringf("comparing %lld and %lld for CMP_LESS_THAN", value0, value1));
        }
        if (operation == CMP_LESS_EQUAL)
        {
            cpu->cv = (value0 <= value1) ? 1 : 0;
            LOGDEBUG(stringf("comparing %lld and %lld for CMP_LESS_EQUAL", value0, value1));
        }
        if (operation == CMP_GREATER_THAN)
        {
            cpu->cv = (value0 > value1) ? 1 : 0;
            LOGDEBUG(stringf("comparing %lld and %lld for CMP_GREATER_THAN", value0, value1));
        }
        if (operation == CMP_GREATER_EQUAL)
        {
            cpu->cv = (value0 >= value1) ? 1 : 0;
            LOGDEBUG(stringf("comparing %lld and %lld for CMP_GREATER_EQUAL", value0, value1));
        }
    }

    LOGDEBUG(stringf("comparison result: %s", cpu->cv ? "true" : "false"));

    if (val1.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val1.value.ptr);
    if (val0.type == BARANIUM_VARIABLE_TYPE_STRING)
        free(val0.value.ptr);
}

void CMPC(bcpu* cpu)
{
    if (!cpu) return;
    if (!cpu->flags.CMP) return;

    uint8_t operation = cpu->fetch(cpu, 8);

    POPCV(cpu);
    uint8_t val0 = cpu->cv;
    POPCV(cpu);
    uint8_t val1 = cpu->cv;

    LOGDEBUG(stringf("comparing CV{%d} and CV{%d} using %s", val0, val1, operation == CMP_AND ? "CMP_AND" : "CMP_OR"));

    if (operation == CMP_AND && val0 == val1 && val1 == 1)
        cpu->cv = 1;
    else if (operation == CMP_OR && (val0 == 1 || val1 == 1))
        cpu->cv = 1;
    else
        cpu->cv = 0;
}

void MEM(bcpu* cpu)
{
    if (!cpu) return;

    bvarmgr* varmgr = cpu->runtime->varmgr;

    size_t size = cpu->fetch(cpu, 64);
    baranium_variable_type_t type = cpu->fetch(cpu, 8);
    index_t id = cpu->fetch(cpu, 64);

    bvarmgr_alloc(varmgr, type, id, size, false);
}

void FEM(bcpu* cpu)
{
    if (!cpu) return;

    bvarmgr* varmgr = cpu->runtime->varmgr;

    index_t id = cpu->fetch(cpu, 64);
    bvarmgr_dealloc(varmgr, id);
}

void SET(bcpu* cpu)
{
    if (!cpu) return;

    bvarmgr* varmgr = cpu->runtime->varmgr;

    index_t id = cpu->fetch(cpu, 64);
    size_t size = cpu->fetch(cpu, 64);

    bvarmgr_n* entry = bvarmgr_get(varmgr, id);
    if (!entry)
    {
        bstack_push(cpu->stack, BARANIUM_ERROR_VAR_NOT_FOUND);
        cpu->flags.FORCED_KILL = true;
        cpu->kill_triggered = true;
        return;
    }
    baranium_variable* var = entry->variable;

    if (var->size != size)
    {
        bstack_push(cpu->stack, BARANIUM_ERROR_VAR_WRONG_SIZE);
        cpu->flags.FORCED_KILL = true;
        cpu->kill_triggered = true;
        return;
    }

    uint8_t* varData = (uint8_t*)&var->value.num64;
    if (var->type == BARANIUM_VARIABLE_TYPE_STRING)
        varData = var->value.ptr;

    for (size_t i = 0; i < size; i++)
    {
        uint8_t fetched = cpu->fetch(cpu, 8);
        *varData = fetched;
        varData++;
    }
}

extern internal_operation_t instantiate_callback;
extern internal_operation_t delete_callback;
extern internal_operation_t attach_callback;
extern internal_operation_t detach_callback;

void INSTANTIATE(bcpu* cpu)
{
    if (!cpu) return;

    index_t id = bstack_pop(cpu->stack);
    if (instantiate_callback) instantiate_callback(id);
    LOGDEBUG(stringf("Object with id '%ld' instantiated", id));
}

void DELETE(bcpu* cpu)
{
    if (!cpu) return;

    index_t id = bstack_pop(cpu->stack);
    if (delete_callback) delete_callback(id);
    LOGDEBUG(stringf("Object with id '%ld' deleted", id));
}

void ATTACH(bcpu* cpu)
{
    if (!cpu) return;

    index_t id = bstack_pop(cpu->stack);
    if (attach_callback) attach_callback(id);
    LOGDEBUG(stringf("Attached to object with id '%ld'", id));
}

void DETACH(bcpu* cpu)
{
    if (!cpu) return;

    index_t id = bstack_pop(cpu->stack);
    if (detach_callback) detach_callback(id);
    LOGDEBUG(stringf("Detached from object with id '%ld'", id));
}

void KILL(bcpu* cpu)
{
    if (!cpu) return;

    int64_t errorCode = cpu->fetch(cpu, 64);
    bstack_push(cpu->stack, errorCode);
    cpu->flags.FORCED_KILL = true;
    cpu->kill_triggered = true;
}
