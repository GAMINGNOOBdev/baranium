#include <baranium/logging.h>
#include <baranium/bcpu.h>
#include <stdbool.h>

void INVALID_OPCODE(bcpu* cpu)
{
    cpu->killTriggered = true;
    LOGERROR("invalid opcode, quitting...");
}

void NOP(bcpu* cpu) {}

void CCF(bcpu* cpu)
{
    cpu->flags.CMP = false;
}

void SCF(bcpu* cpu)
{
    cpu->flags.CMP = true;
}

void CCV(bcpu* cpu)
{
    cpu->cv = 0;
}

void PUSHCV(bcpu* cpu)
{
    cpu->cv_stack.push(&cpu->cv_stack, cpu->cv);
}

void POPCV(bcpu* cpu)
{
    cpu->cv = cpu->cv_stack.pop(&cpu->cv_stack);
}

void PUSHIP(bcpu* cpu)
{
    cpu->ip_stack.push(&cpu->ip_stack, cpu->IP);
}

void POPIP(bcpu* cpu)
{
    cpu->IP = cpu->ip_stack.pop(&cpu->ip_stack);
}

void JMP(bcpu* cpu)
{
    uint64_t addr = cpu->fetch(cpu, 64);
    cpu->IP = addr;
}

void JMPOFF(bcpu* cpu)
{
    int16_t offset = cpu->fetch(cpu, 16);
    cpu->IP += offset;
}

void JEQ(bcpu* cpu)
{
    uint64_t addr = cpu->fetch(cpu, 64);
    if (!cpu->flags.CMP || cpu->cv == 0)
        return;

    cpu->IP = addr;
}

void JEQOFF(bcpu* cpu)
{
    uint16_t offset = cpu->fetch(cpu, 16);
    if (!cpu->flags.CMP || cpu->cv == 0)
        return;

    cpu->IP += offset;
}

void JNQ(bcpu* cpu)
{
    uint64_t addr = cpu->fetch(cpu, 64);
    if (!cpu->flags.CMP || cpu->cv != 0)
        return;

    cpu->IP = addr;
}

void JNQOFF(bcpu* cpu)
{
    uint16_t offset = cpu->fetch(cpu, 16);
    if (!cpu->flags.CMP || cpu->cv != 0)
        return;

    cpu->IP += offset;
}

void JLZ(bcpu* cpu)
{
    uint64_t addr = cpu->fetch(cpu, 64);
    if (!cpu->flags.CMP || cpu->cv < 0)
        return;

    cpu->IP = addr;
}

void JLZOFF(bcpu* cpu)
{
    uint16_t offset = cpu->fetch(cpu, 16);
    if (!cpu->flags.CMP || cpu->cv < 0)
        return;

    cpu->IP += offset;
}

void JGZ(bcpu* cpu)
{
    uint64_t addr = cpu->fetch(cpu, 64);
    if (!cpu->flags.CMP || cpu->cv > 0)
        return;

    cpu->IP = addr;
}

void JGZOFF(bcpu* cpu)
{
    uint16_t offset = cpu->fetch(cpu, 16);
    if (!cpu->flags.CMP || cpu->cv > 0)
        return;

    cpu->IP += offset;
}

void MEM(bcpu* cpu) {}

void FEM(bcpu* cpu) {}

void SET(bcpu* cpu) {}