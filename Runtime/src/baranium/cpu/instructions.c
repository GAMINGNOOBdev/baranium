#include <baranium/backend/bfuncmgr.h>
#include <baranium/backend/bvarmgr.h>
#include <baranium/backend/varmath.h>
#include <baranium/backend/errors.h>
#include <baranium/variable.h>
#include <baranium/callback.h>
#include <baranium/runtime.h>
#include <baranium/logging.h>
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

    cpu->killTriggered = true;
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
    var.type = VARIABLE_TYPE_BOOL;
    var.value = &cpu->cv;
    baranium_compiled_variable_push_to_stack(cpu, &var);
}

void POPCV(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable* var = baranium_compiled_variable_pop_from_stack(cpu);
    cpu->cv = *((uint8_t*)var->value);
    free(var->value);
    baranium_compiled_variable_dispose(var);
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
        bstack_push(cpu->stack, ERR_VAR_NOT_FOUND);
        cpu->flags.FORCED_KILL = true;
        cpu->killTriggered = true;
        return;
    }

    size_t size = 0;
    void* value = NULL;
    baranium_variable_type_t type = VARIABLE_TYPE_INVALID;
    if (var->isVariable)
    {
        size = var->variable->Size;
        type = var->variable->Type;
        value = var->variable->Value;
    }
    else
    {
        size = var->field->Size;
        type = var->field->Type;
        value = var->field->Value;
    }

    if (type == VARIABLE_TYPE_VOID || type == VARIABLE_TYPE_INVALID)
    {
        LOGERROR(stringf("Variable/Field with ID '%d' cannot be pushed: Invalid type", id));
        bstack_push(cpu->stack, ERR_VAR_INVALID_TYPE);
        cpu->flags.FORCED_KILL = true;
        cpu->killTriggered = true;
        return;
    }

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

void POPVAR(bcpu* cpu)
{
    if (!cpu) return;

    index_t id = (index_t)cpu->fetch(cpu, 64);
    bvarmgr* varmgr = cpu->runtime->varmgr;
    bvarmgr_n* var = bvarmgr_get(varmgr, id);
    if (var == NULL)
    {
        LOGERROR(stringf("Variable/Field with ID '%d' not found", id));
        bstack_push(cpu->stack, ERR_VAR_NOT_FOUND);
        cpu->flags.FORCED_KILL = true;
        cpu->killTriggered = true;
        return;
    }

    void* value = NULL;
    baranium_variable_type_t type = VARIABLE_TYPE_INVALID;
    if (var->isVariable)
    {
        type = var->variable->Type;
        value = var->variable->Value;
    }
    else
    {
        type = var->field->Type;
        value = var->field->Value;
    }

    if (type == VARIABLE_TYPE_VOID || type == VARIABLE_TYPE_INVALID)
    {
        LOGERROR(stringf("Variable/Field with ID '%d' cannot be assigned: Invalid type", id));
        bstack_push(cpu->stack, ERR_VAR_INVALID_TYPE);
        cpu->flags.FORCED_KILL = true;
        cpu->killTriggered = true;
        return;
    }

    baranium_compiled_variable* newvar = baranium_compiled_variable_pop_from_stack(cpu);
    baranium_compiled_variable_convert_to_type(newvar, type);
    if (!baranium_variable_type_size_interchangable(type, newvar->type))
    {
        LOGERROR(stringf("Variable/Field with ID '%d' cannot be assigned: Non-matching types of variable and assign value", id));
        bstack_push(cpu->stack, ERR_VAR_INVALID_TYPE);
        free(newvar->value);
        baranium_compiled_variable_dispose(newvar);
        cpu->flags.FORCED_KILL = true;
        cpu->killTriggered = true;
        return;
    }

    // not needed anymore since we'd overwrite the data
    if (value)
        free(value);

    // instead of overwriting (and potentially
    // having to reallocate memory anyways) just
    // allocate a new value pointer

    if (type == VARIABLE_TYPE_STRING && newvar->type != VARIABLE_TYPE_STRING)
    {
        char* stringifiedVersion = baranium_variable_stringify(newvar->type, newvar->value);
        size_t stringSize = strlen(stringifiedVersion);
        if (stringifiedVersion != newvar->value)
        {
            free(newvar->value);
            newvar->value = malloc(stringSize+1);
            memset(newvar->value, 0, stringSize+1);
            memcpy(newvar->value, stringifiedVersion, stringSize);
        }
    }

    // very important step to get an expected result
    baranium_compiled_variable_convert_to_type(newvar, type);

    // assign new size and value pointers for the variable/field
    if (var->isVariable)
    {
        var->variable->Size = newvar->size;
        var->variable->Value = newvar->value;
    }
    else
    {
        var->field->Size = newvar->size;
        var->field->Value = newvar->value;
    }

    baranium_compiled_variable_dispose(newvar);
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
    bstack_push(cpu->ip_stack, cpu->IP);

    LOGDEBUG(stringf("calling function with id '%lld' (current IP: %lld)", id, cpu->IP));

    baranium_runtime* runtime = baranium_get_context();
    baranium_callback_list_entry* callback = baranium_callback_find_by_id( id);
    LOGDEBUG(stringf("callback id: %lld callback ptr: 0x%16.16x", id, (uint64_t)callback));
    baranium_function* func = baranium_function_manager_get(runtime->functionManager, id);

    if (callback != NULL)
    {
        int numParams = callback->numParams;
        void** dataptr = NULL;
        baranium_variable_type_t* datatypes = NULL;
        if (numParams > 0 && numParams != -1)
        {
            dataptr = malloc(sizeof(void*)*numParams);
            datatypes = malloc(sizeof(baranium_variable_type_t));

            for (int i = 0; i < numParams; i++)
            {
                baranium_compiled_variable* tmp = baranium_compiled_variable_pop_from_stack(cpu);
                dataptr[i] = tmp->value;
                datatypes[i] = tmp->type;
                baranium_compiled_variable_dispose(tmp);
            }
        }
        LOGDEBUG(stringf("callback call: dataptr{0x%16.16x} datatypes{0x%16.16x} numParams=%d", (uint64_t)dataptr, (uint64_t)datatypes, numParams));
        callback->callback(dataptr, datatypes, numParams);
        if (numParams > 0 && numParams != -1)
        {
            for (int i = 0; i < numParams; i++)
                free(dataptr[i]);
            free(dataptr);
            free(datatypes);
        }
    }
    else
        baranium_function_call(func, NULL, NULL, -1);

    baranium_function_dispose(func);

    cpu->IP = bstack_pop(cpu->ip_stack);
    LOGDEBUG(stringf("finished calling function with id '%lld' (current IP: %lld)", id, cpu->IP));
}

void RET(bcpu* cpu)
{
    if (!cpu) return;

    cpu->killTriggered = 1;
}

void JMP(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t addr = cpu->fetch(cpu, 64);
    cpu->IP = addr;
}

void JMPOFF(bcpu* cpu)
{
    if (!cpu) return;

    int16_t offset = cpu->fetch(cpu, 16);
    cpu->IP += offset;
}

void JMPC(bcpu* cpu)
{
    if (!cpu) return;
    
    uint64_t addr = cpu->fetch(cpu, 64);
    if (!cpu->flags.CMP) return;
    if (cpu->cv == 0)
        return;

    cpu->IP = addr;
}

void JMPCOFF(bcpu* cpu)
{
    if (!cpu) return;
    
    uint16_t offset = cpu->fetch(cpu, 16);
    if (!cpu->flags.CMP) return;
    if (cpu->cv == 0)
        return;

    cpu->IP += offset;
}

void MOD(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable* divisor = baranium_compiled_variable_pop_from_stack(cpu);
    baranium_compiled_variable* divident = baranium_compiled_variable_pop_from_stack(cpu);

    baranium_compiled_variable_combine(divisor, divident, BARANIUM_VARIABLE_OPERATION_MOD, divisor->type);

    baranium_compiled_variable_push_to_stack(cpu, divisor);
    free(divident->value);
    free(divisor->value);
    baranium_compiled_variable_dispose(divident);
    baranium_compiled_variable_dispose(divisor);
}

void DIV(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable* divisor = baranium_compiled_variable_pop_from_stack(cpu);
    baranium_compiled_variable* divident = baranium_compiled_variable_pop_from_stack(cpu);

    baranium_compiled_variable_combine(divisor, divident, BARANIUM_VARIABLE_OPERATION_DIV, divisor->type);

    baranium_compiled_variable_push_to_stack(cpu, divisor);
    free(divident->value);
    free(divisor->value);
    baranium_compiled_variable_dispose(divident);
    baranium_compiled_variable_dispose(divisor);
}

void MUL(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable* val0 = baranium_compiled_variable_pop_from_stack(cpu);
    baranium_compiled_variable* val1 = baranium_compiled_variable_pop_from_stack(cpu);

    baranium_compiled_variable_combine(val0, val1, BARANIUM_VARIABLE_OPERATION_MUL, val0->type);

    baranium_compiled_variable_push_to_stack(cpu, val0);
    free(val1->value);
    free(val0->value);
    baranium_compiled_variable_dispose(val1);
    baranium_compiled_variable_dispose(val0);
}

void SUB(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable* val0 = baranium_compiled_variable_pop_from_stack(cpu);
    baranium_compiled_variable* val1 = baranium_compiled_variable_pop_from_stack(cpu);

    baranium_compiled_variable_combine(val0, val1, BARANIUM_VARIABLE_OPERATION_SUB, val0->type);

    baranium_compiled_variable_push_to_stack(cpu, val0);
    free(val1->value);
    free(val0->value);
    baranium_compiled_variable_dispose(val1);
    baranium_compiled_variable_dispose(val0);
}

void ADD(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable* val0 = baranium_compiled_variable_pop_from_stack(cpu);
    baranium_compiled_variable* val1 = baranium_compiled_variable_pop_from_stack(cpu);

    baranium_compiled_variable_combine(val0, val1, BARANIUM_VARIABLE_OPERATION_ADD, val0->type);

    baranium_compiled_variable_push_to_stack(cpu, val0);
    free(val1->value);
    free(val0->value);
    baranium_compiled_variable_dispose(val1);
    baranium_compiled_variable_dispose(val0);
}

void AND(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable* val0 = baranium_compiled_variable_pop_from_stack(cpu);
    baranium_compiled_variable* val1 = baranium_compiled_variable_pop_from_stack(cpu);

    baranium_compiled_variable_combine(val0, val1, BARANIUM_VARIABLE_OPERATION_AND, val0->type);

    baranium_compiled_variable_push_to_stack(cpu, val0);
    free(val1->value);
    free(val0->value);
    baranium_compiled_variable_dispose(val1);
    baranium_compiled_variable_dispose(val0);
}

void OR(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable* val0 = baranium_compiled_variable_pop_from_stack(cpu);
    baranium_compiled_variable* val1 = baranium_compiled_variable_pop_from_stack(cpu);

    baranium_compiled_variable_combine(val0, val1, BARANIUM_VARIABLE_OPERATION_OR, val0->type);

    baranium_compiled_variable_push_to_stack(cpu, val0);
    free(val1->value);
    free(val0->value);
    baranium_compiled_variable_dispose(val1);
    baranium_compiled_variable_dispose(val0);
}

void XOR(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable* val0 = baranium_compiled_variable_pop_from_stack(cpu);
    baranium_compiled_variable* val1 = baranium_compiled_variable_pop_from_stack(cpu);

    baranium_compiled_variable_combine(val0, val1, BARANIUM_VARIABLE_OPERATION_XOR, val0->type);

    baranium_compiled_variable_push_to_stack(cpu, val0);
    free(val1->value);
    free(val0->value);
    baranium_compiled_variable_dispose(val1);
    baranium_compiled_variable_dispose(val0);
}

void SHFTL(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable* val0 = baranium_compiled_variable_pop_from_stack(cpu);
    baranium_compiled_variable* val1 = baranium_compiled_variable_pop_from_stack(cpu);

    baranium_compiled_variable_combine(val0, val1, BARANIUM_VARIABLE_OPERATION_SHFTL, val0->type);

    baranium_compiled_variable_push_to_stack(cpu, val0);
    free(val1->value);
    free(val0->value);
    baranium_compiled_variable_dispose(val1);
    baranium_compiled_variable_dispose(val0);
}

void SHFTR(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable* val0 = baranium_compiled_variable_pop_from_stack(cpu);
    baranium_compiled_variable* val1 = baranium_compiled_variable_pop_from_stack(cpu);

    baranium_compiled_variable_combine(val0, val1, BARANIUM_VARIABLE_OPERATION_SHFTR, val0->type);

    baranium_compiled_variable_push_to_stack(cpu, val0);
    free(val1->value);
    free(val0->value);
    baranium_compiled_variable_dispose(val1);
    baranium_compiled_variable_dispose(val0);
}

void CMP(bcpu* cpu)
{
    if (!cpu) return;
    if (!cpu->flags.CMP) return;

    uint8_t operation = cpu->fetch(cpu, 8);

    baranium_compiled_variable* val1 = baranium_compiled_variable_pop_from_stack(cpu);
    baranium_compiled_variable* val0 = baranium_compiled_variable_pop_from_stack(cpu);

    // yes this is a somewhat lazy way but hey, it's somewhat logical as well so shut up
    if (val1->type == VARIABLE_TYPE_STRING || val0->type == VARIABLE_TYPE_STRING)
    {
        baranium_compiled_variable_convert_to_type(val0, VARIABLE_TYPE_STRING);
        char* value0 = (char*)val0->value;
        baranium_compiled_variable_convert_to_type(val1, VARIABLE_TYPE_STRING);
        char* value1 = (char*)val1->value;

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
    else if (val1->type == VARIABLE_TYPE_FLOAT || val0->type == VARIABLE_TYPE_FLOAT)
    {
        baranium_compiled_variable_convert_to_type(val0, VARIABLE_TYPE_FLOAT);
        float value0 = *(float*)val0->value;
        baranium_compiled_variable_convert_to_type(val1, VARIABLE_TYPE_FLOAT);
        float value1 = *(float*)val1->value;

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
        baranium_compiled_variable_convert_to_type(val0, VARIABLE_TYPE_OBJECT);
        int64_t value0 = *(int64_t*)val0->value;
        baranium_compiled_variable_convert_to_type(val1, VARIABLE_TYPE_OBJECT);
        int64_t value1 = *(int64_t*)val1->value;

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

    free(val0->value);
    free(val1->value);
    baranium_compiled_variable_dispose(val0);
    baranium_compiled_variable_dispose(val1);
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
        bstack_push(cpu->stack, ERR_VAR_NOT_FOUND);
        cpu->flags.FORCED_KILL = true;
        cpu->killTriggered = true;
        return;
    }
    baranium_variable* var = entry->variable;

    if (var->Size != size)
    {
        bstack_push(cpu->stack, ERR_VAR_WRONG_SIZE);
        cpu->flags.FORCED_KILL = true;
        cpu->killTriggered = true;
        return;
    }

    uint8_t* varData = (uint8_t*)var->Value;

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
    cpu->killTriggered = true;
}
