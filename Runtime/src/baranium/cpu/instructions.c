#include <baranium/backend/errors.h>
#include <baranium/logging.h>
#include <baranium/bcpu.h>
#include <stdbool.h>

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

    bstack_push(cpu->stack, cpu->cv);
}

void POPCV(bcpu* cpu)
{
    if (!cpu) return;

    cpu->cv = bstack_pop(cpu->stack);
}

void PUSHVAR(bcpu* cpu)
{
    if (!cpu) return;

    // index_t id = cpu->fetch(cpu, 64);
    ///TODO: actually push the variable value
    bstack_push(cpu->stack, 0);
}

void POPVAR(bcpu* cpu)
{
    if (!cpu) return;
    
    // index_t id = cpu->fetch(cpu, 64);
    ///TODO: actually assign the variable value
    bstack_pop(cpu->stack);
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

    uint64_t divisor = bstack_pop(cpu->stack);
    uint64_t divident = bstack_pop(cpu->stack);

    if (divident == 0)
    {
        cpu->killTriggered = 1;
        cpu->flags.FORCED_KILL = 1;
        return;
    }

    bstack_push(cpu->stack, divisor % divident);
}

void DIV(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t divisor = bstack_pop(cpu->stack);
    uint64_t divident = bstack_pop(cpu->stack);

    if (divisor == 0)
    {
        cpu->killTriggered = 1;
        cpu->flags.FORCED_KILL = 1;
        return;
    }

    bstack_push(cpu->stack, divident / divisor);
}

void MUL(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t val0 = bstack_pop(cpu->stack);
    uint64_t val1 = bstack_pop(cpu->stack);

    bstack_push(cpu->stack, val0 * val1);
}

void SUB(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t subtrahend = bstack_pop(cpu->stack);
    uint64_t minuend = bstack_pop(cpu->stack);

    bstack_push(cpu->stack, minuend - subtrahend);
}

void ADD(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t val0 = bstack_pop(cpu->stack);
    uint64_t val1 = bstack_pop(cpu->stack);

    bstack_push(cpu->stack, val0 + val1);
}

void AND(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t val0 = bstack_pop(cpu->stack);
    uint64_t val1 = bstack_pop(cpu->stack);

    bstack_push(cpu->stack, val0 & val1);
}

void OR(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t val0 = bstack_pop(cpu->stack);
    uint64_t val1 = bstack_pop(cpu->stack);

    bstack_push(cpu->stack, val0 | val1);
}

void XOR(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t val0 = bstack_pop(cpu->stack);
    uint64_t val1 = bstack_pop(cpu->stack);

    bstack_push(cpu->stack, val0 ^ val1);
}

void SHFTL(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t val0 = bstack_pop(cpu->stack);
    uint64_t val1 = bstack_pop(cpu->stack);

    bstack_push(cpu->stack, val0 << val1);
}

void SHFTR(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t val0 = bstack_pop(cpu->stack);
    uint64_t val1 = bstack_pop(cpu->stack);

    bstack_push(cpu->stack, val0 >> val1);
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

    size_t size = cpu->fetch(cpu, 64);
    enum BaraniumVariableType type = cpu->fetch(cpu, 8);
    index_t id = cpu->fetch(cpu, 64);

    bvarmgr_alloc(cpu->varmgr, type, id, size);
}

void FEM(bcpu* cpu)
{
    if (!cpu) return;

    index_t id = cpu->fetch(cpu, 64);
    bvarmgr_dealloc(cpu->varmgr, id);
}

void SET(bcpu* cpu)
{
    if (!cpu) return;

    index_t id = cpu->fetch(cpu, 64);
    size_t size = cpu->fetch(cpu, 64);

    LOGDEBUG(stringf("Setting variable with id '%ld' and size '%ld'", id, size));

    BaraniumVariable* var = bvarmgr_get(cpu->varmgr, id);
    if (!var)
    {
        bstack_push(cpu->stack, ERR_NO_VAR_FOUND);
        cpu->flags.FORCED_KILL = true;
        cpu->killTriggered = true;
        return;
    }

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