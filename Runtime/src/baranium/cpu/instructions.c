#include <baranium/logging.h>
#include <baranium/bcpu.h>
#include <stdbool.h>

void INVALID_OPCODE(bcpu* cpu)
{
    if (!cpu)
        return;

    cpu->killTriggered = true;
    LOGERROR("invalid opcode, quitting...");
}

void NOP(bcpu* cpu) {}

void CCF(bcpu* cpu)
{
    if (!cpu)
        return;

    cpu->flags.CMP = false;
}

void SCF(bcpu* cpu)
{
    if (!cpu)
        return;

    cpu->flags.CMP = true;
}

void CCV(bcpu* cpu)
{
    if (!cpu)
        return;

    cpu->cv = 0;
}

void PUSHCV(bcpu* cpu)
{
    if (!cpu)
        return;

    cpu->cv_stack.push(&cpu->cv_stack, cpu->cv);
}

void POPCV(bcpu* cpu)
{
    if (!cpu)
        return;

    cpu->cv = cpu->cv_stack.pop(&cpu->cv_stack);
}

void PUSHIP(bcpu* cpu)
{
    if (!cpu)
        return;

    cpu->ip_stack.push(&cpu->ip_stack, cpu->IP);
}

void POPIP(bcpu* cpu)
{
    if (!cpu)
        return;

    cpu->IP = cpu->ip_stack.pop(&cpu->ip_stack);
}

void JMP(bcpu* cpu)
{
    if (!cpu)
        return;

    uint64_t addr = cpu->fetch(cpu, 64);
    cpu->IP = addr;
}

void JMPOFF(bcpu* cpu)
{
    if (!cpu)
        return;

    int16_t offset = cpu->fetch(cpu, 16);
    cpu->IP += offset;
}

void JEQ(bcpu* cpu)
{
    if (!cpu)
        return;

    uint64_t addr = cpu->fetch(cpu, 64);
    if (!cpu->flags.CMP || cpu->cv == 0)
        return;

    cpu->IP = addr;
}

void JEQOFF(bcpu* cpu)
{
    if (!cpu)
        return;

    uint16_t offset = cpu->fetch(cpu, 16);
    if (!cpu->flags.CMP || cpu->cv == 0)
        return;

    cpu->IP += offset;
}

void JNQ(bcpu* cpu)
{
    if (!cpu)
        return;

    uint64_t addr = cpu->fetch(cpu, 64);
    if (!cpu->flags.CMP || cpu->cv != 0)
        return;

    cpu->IP = addr;
}

void JNQOFF(bcpu* cpu)
{
    if (!cpu)
        return;

    uint16_t offset = cpu->fetch(cpu, 16);
    if (!cpu->flags.CMP || cpu->cv != 0)
        return;

    cpu->IP += offset;
}

void JLZ(bcpu* cpu)
{
    if (!cpu)
        return;

    uint64_t addr = cpu->fetch(cpu, 64);
    if (!cpu->flags.CMP || cpu->cv < 0)
        return;

    cpu->IP = addr;
}

void JLZOFF(bcpu* cpu)
{
    if (!cpu)
        return;

    uint16_t offset = cpu->fetch(cpu, 16);
    if (!cpu->flags.CMP || cpu->cv < 0)
        return;

    cpu->IP += offset;
}

void JGZ(bcpu* cpu)
{
    if (!cpu)
        return;

    uint64_t addr = cpu->fetch(cpu, 64);
    if (!cpu->flags.CMP || cpu->cv > 0)
        return;

    cpu->IP = addr;
}

void JGZOFF(bcpu* cpu)
{
    if (!cpu)
        return;

    uint16_t offset = cpu->fetch(cpu, 16);
    if (!cpu->flags.CMP || cpu->cv > 0)
        return;

    cpu->IP += offset;
}

void MEM(bcpu* cpu)
{
    if (!cpu)
        return;

    uint64_t size = cpu->fetch(cpu, 64);
    uint64_t id = cpu->fetch(cpu, 64);

    // do some allocation stuff
}

void FEM(bcpu* cpu)
{
    if (!cpu)
        return;

    uint64_t id = cpu->fetch(cpu, 64);

    // do some deallocation stuff
}

void SET(bcpu* cpu)
{
    if (!cpu)
        return;

    uint64_t id = cpu->fetch(cpu, 64);
    uint64_t size = cpu->fetch(cpu, 64);

    for (uint64_t i = 0; i < size; i++)
    {
        cpu->fetch(cpu, 8); // do smth with the fetched data
    }
}

void KILL(bcpu* cpu)
{
    if (!cpu)
        return;

    int64_t errorCode = cpu->fetch(cpu, 64);
    cpu->stack.push(&cpu->stack, errorCode);
    cpu->flags.FORCED_KILL = true;
    cpu->killTriggered = true;
}