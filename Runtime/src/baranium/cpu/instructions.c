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

    bstack_push(&cpu->cv_stack, cpu->cv);
}

void POPCV(bcpu* cpu)
{
    if (!cpu) return;

    cpu->cv = bstack_pop(&cpu->cv_stack);
}

void PUSHVAR(bcpu* cpu)
{
    if (!cpu) return;

    // index_t id = cpu->fetch(cpu, 64);
    ///TODO: actually push the variable value
    bstack_push(&cpu->stack, 0);
}

void POPVAR(bcpu* cpu)
{
    if (!cpu) return;
    
    // index_t id = cpu->fetch(cpu, 64);
    ///TODO: actually assign the variable value
    bstack_pop(&cpu->stack);
}

void PUSH(bcpu* cpu)
{
    if (!cpu) return;
    
    uint64_t value = cpu->fetch(cpu, 64);
    bstack_push(&cpu->stack, value);
}

void PUSHIP(bcpu* cpu)
{
    if (!cpu) return;

    bstack_push(&cpu->ip_stack, cpu->IP);
}

void POPIP(bcpu* cpu)
{
    if (!cpu) return;

    cpu->IP = bstack_pop(&cpu->ip_stack);
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

    uint64_t val0 = bstack_pop(&cpu->stack);
    uint64_t val1 = bstack_pop(&cpu->stack);

    if (val1 == 0)
    {
        cpu->killTriggered = 1;
        cpu->flags.FORCED_KILL = 1;
        return;
    }

    bstack_push(&cpu->stack, val0 % val1);
}

void DIV(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t val0 = bstack_pop(&cpu->stack);
    uint64_t val1 = bstack_pop(&cpu->stack);

    if (val1 == 0)
    {
        cpu->killTriggered = 1;
        cpu->flags.FORCED_KILL = 1;
        return;
    }

    bstack_push(&cpu->stack, val0 / val1);
}

void MUL(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t val0 = bstack_pop(&cpu->stack);
    uint64_t val1 = bstack_pop(&cpu->stack);

    bstack_push(&cpu->stack, val0 * val1);
}

void SUB(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t val0 = bstack_pop(&cpu->stack);
    uint64_t val1 = bstack_pop(&cpu->stack);

    bstack_push(&cpu->stack, val0 - val1);
}

void ADD(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t val0 = bstack_pop(&cpu->stack);
    uint64_t val1 = bstack_pop(&cpu->stack);

    bstack_push(&cpu->stack, val0 + val1);
}

void AND(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t val0 = bstack_pop(&cpu->stack);
    uint64_t val1 = bstack_pop(&cpu->stack);

    bstack_push(&cpu->stack, val0 & val1);
}

void OR(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t val0 = bstack_pop(&cpu->stack);
    uint64_t val1 = bstack_pop(&cpu->stack);

    bstack_push(&cpu->stack, val0 | val1);
}

void XOR(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t val0 = bstack_pop(&cpu->stack);
    uint64_t val1 = bstack_pop(&cpu->stack);

    bstack_push(&cpu->stack, val0 ^ val1);
}

void SHFTL(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t val0 = bstack_pop(&cpu->stack);
    uint64_t val1 = bstack_pop(&cpu->stack);

    bstack_push(&cpu->stack, val0 << val1);
}

void SHFTR(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t val0 = bstack_pop(&cpu->stack);
    uint64_t val1 = bstack_pop(&cpu->stack);

    bstack_push(&cpu->stack, val0 >> val1);
}

void MEM(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t size = cpu->fetch(cpu, 64);
    uint64_t id = cpu->fetch(cpu, 64);

    // do some allocation stuff
}

void FEM(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t id = cpu->fetch(cpu, 64);

    // do some deallocation stuff
}

void SET(bcpu* cpu)
{
    if (!cpu) return;

    uint64_t id = cpu->fetch(cpu, 64);
    uint64_t size = cpu->fetch(cpu, 64);

    for (uint64_t i = 0; i < size; i++)
    {
        cpu->fetch(cpu, 8); // do smth with the fetched data
    }
}

void KILL(bcpu* cpu)
{
    if (!cpu) return;

    int64_t errorCode = cpu->fetch(cpu, 64);
    bstack_push(&cpu->stack, errorCode);
    cpu->flags.FORCED_KILL = true;
    cpu->killTriggered = true;
}