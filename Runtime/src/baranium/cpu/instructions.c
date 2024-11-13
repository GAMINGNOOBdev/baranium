#include <baranium/backend/varmath.h>
#include <baranium/backend/errors.h>
#include <baranium/logging.h>
#include <baranium/bcpu.h>
#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>

void INVALID_OPCODE(bcpu* cpu)
{
    if (!cpu) return;

    cpu->killTriggered = true;
    LOGERROR("invalid opcode, quitting...");
}

void NOP(bcpu* cpu) {}

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

void PUSHCV(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable var;
    var.size = sizeof(int64_t);
    var.type = BaraniumVariableType_Int;
    var.value = &cpu->cv;
    baranium_compiled_variable_push_to_stack(cpu, &var);
}

void POPCV(bcpu* cpu)
{
    if (!cpu) return;

    baranium_compiled_variable* var = baranium_compiled_variable_pop_from_stack(cpu);
    cpu->cv = *((int64_t*)var->value);
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
    enum BaraniumVariableType type = BaraniumVariableType_Invalid;
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

    if (type == BaraniumVariableType_Void || type == BaraniumVariableType_Invalid)
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

    size_t size = 0;
    void* value = NULL;
    enum BaraniumVariableType type = BaraniumVariableType_Invalid;
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

    if (type == BaraniumVariableType_Void || type == BaraniumVariableType_Invalid)
    {
        LOGERROR(stringf("Variable/Field with ID '%d' cannot be assigned: Invalid type", id));
        bstack_push(cpu->stack, ERR_VAR_INVALID_TYPE);
        cpu->flags.FORCED_KILL = true;
        cpu->killTriggered = true;
        return;
    }

    baranium_compiled_variable* newvar = baranium_compiled_variable_pop_from_stack(cpu);
    if (!baranium_variable_type_size_interchangable(type, newvar->type))
    {
        LOGERROR(stringf("Variable/Field with ID '%d' cannot be assigned: Non-matching types of variable and assign value", id));
        bstack_push(cpu->stack, ERR_VAR_INVALID_TYPE);
        cpu->flags.FORCED_KILL = true;
        cpu->killTriggered = true;
        return;
    }

    // not needed anymore since we'd overwrite the data
    if (value)
        free(value);

    // instead of overwriting (and potentially
    // having to reallocate memory andways) just
    // allocate a new value pointer

    if (type == BaraniumVariableType_String && newvar->type != BaraniumVariableType_String)
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

    bstack_push(cpu->ip_stack, cpu->IP);
}

void RET(bcpu* cpu)
{
    if (!cpu) return;

    cpu->IP = bstack_pop(cpu->ip_stack);
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

void JEQ(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t addr = cpu->fetch(cpu, 64);
    if (!cpu->flags.CMP || cpu->cv == 0)
        return;

    cpu->IP = addr;
}

void JEQOFF(bcpu* cpu)
{
    if (!cpu) return;

    uint16_t offset = cpu->fetch(cpu, 16);
    if (!cpu->flags.CMP || cpu->cv == 0)
        return;

    cpu->IP += offset;
}

void JNQ(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t addr = cpu->fetch(cpu, 64);
    if (!cpu->flags.CMP || cpu->cv != 0)
        return;

    cpu->IP = addr;
}

void JNQOFF(bcpu* cpu)
{
    if (!cpu) return;

    uint16_t offset = cpu->fetch(cpu, 16);
    if (!cpu->flags.CMP || cpu->cv != 0)
        return;

    cpu->IP += offset;
}

void JLZ(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t addr = cpu->fetch(cpu, 64);
    if (!cpu->flags.CMP || cpu->cv < 0)
        return;

    cpu->IP = addr;
}

void JLZOFF(bcpu* cpu)
{
    if (!cpu) return;

    uint16_t offset = cpu->fetch(cpu, 16);
    if (!cpu->flags.CMP || cpu->cv < 0)
        return;

    cpu->IP += offset;
}

void JGZ(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t addr = cpu->fetch(cpu, 64);
    if (!cpu->flags.CMP || cpu->cv > 0)
        return;

    cpu->IP = addr;
}

void JGZOFF(bcpu* cpu)
{
    if (!cpu) return;

    uint16_t offset = cpu->fetch(cpu, 16);
    if (!cpu->flags.CMP || cpu->cv > 0)
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

    uint64_t val0 = bstack_pop(cpu->stack);
    uint64_t val1 = bstack_pop(cpu->stack);

    cpu->cv = val0 - val1;
}

void MEM(bcpu* cpu)
{
    if (!cpu) return;

    bvarmgr* varmgr = cpu->runtime->varmgr;

    size_t size = cpu->fetch(cpu, 64);
    enum BaraniumVariableType type = cpu->fetch(cpu, 8);
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

    LOGDEBUG(stringf("Setting variable with id '%ld' and size '%ld'", id, size));

    bvarmgr_n* entry = bvarmgr_get(varmgr, id);
    if (!entry)
    {
        bstack_push(cpu->stack, ERR_VAR_NOT_FOUND);
        cpu->flags.FORCED_KILL = true;
        cpu->killTriggered = true;
        return;
    }
    BaraniumVariable* var = entry->variable;

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

void INSTANTIATE(bcpu* cpu)
{
    if (!cpu) return;

    index_t id = bstack_pop(cpu->stack);
    ///TODO: actually do some stuff for these object handles

    LOGDEBUG(stringf("Object with id '%ld' created", id));
}

void DELETE(bcpu* cpu)
{
    if (!cpu) return;

    index_t id = bstack_pop(cpu->stack);
    ///TODO: actually do some stuff for these object handles

    LOGDEBUG(stringf("Object with id '%ld' deleted", id));
}

void ATTACH(bcpu* cpu)
{
    if (!cpu) return;

    index_t id = bstack_pop(cpu->stack);
    ///TODO: actually do some stuff for these object handles

    LOGDEBUG(stringf("Attached to object with id '%ld'", id));
}

void DETACH(bcpu* cpu)
{
    if (!cpu) return;

    index_t id = bstack_pop(cpu->stack);
    ///TODO: actually do some stuff for these object handles

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