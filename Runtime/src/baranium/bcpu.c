#include <baranium/cpu/bcpu_opcodes.h>
#include <baranium/logging.h>
#include <baranium/bcpu.h>

// function references (all declared at the end of this file)
uint64_t _bcpu_fetch(bcpu* obj, int bits);

// initializes the cpu with the requered function pointers and also sets the bus of the cpu
void bcpu_init(bcpu* obj)
{
    if (obj == NULL)
        return;

    obj->fetched = 0;
    obj->fetch = _bcpu_fetch;
    obj->ticks = 0;
    obj->killTriggered = 0;
    bcpu_reset(obj);
}

void bcpu_cleanup(bcpu* obj)
{
    if (obj == NULL)
        return;

    if (obj->cv_stack.clear)
        obj->cv_stack.clear(&obj->cv_stack);
    
    if (obj->stack.clear)
        obj->stack.clear(&obj->stack);
    
    if (obj->ip_stack.clear)
        obj->ip_stack.clear(&obj->ip_stack);
}

// this is the method which executes the instructions from the IP value forward
void bcpu_tick(bcpu* obj)
{
    if (obj == NULL)
        return;

    if (obj->bus.eof(&obj->bus, obj->IP))
    {
        obj->killTriggered = 1;
        return;
    }

    obj->opcode = obj->bus.read(&obj->bus, obj->IP);
    LOGDEBUG(stringf("IP: 0x%x | Ticks (total): 0x%x | Opcode: 0x%x | Instruction: '%s'",
               obj->IP, obj->ticks, obj->opcode, opcodes[obj->opcode].name));
    opcodes[obj->opcode].handle(obj);
    obj->IP++;
    obj->ticks++;
}

// resets the cpu
void bcpu_reset(bcpu* obj)
{
    if (obj == NULL)
        return;

    obj->IP = 0;
    bstack_init(&obj->stack);
    bstack_init(&obj->ip_stack);
    bstack_init(&obj->cv_stack);
    obj->flags.CMP = 1;
    obj->flags.RESERVED = 0;
    obj->ticks = 0;
    bbus_init(&obj->bus, NULL);
}

uint64_t _bcpu_fetch(bcpu* obj, int bits)
{
    if (obj == NULL)
        return 0;

    if (bits > 64 || bits % 8 != 0)
    {
        obj->killTriggered = 1;
        return;
    }
    int bytes = bits / 8;

    obj->fetched = 0;
    for (int i = 0; i < bytes; i++)
    {
        obj->fetched |= (obj->bus.read(&obj->bus, obj->IP)) << (bytes - i - 1);
        obj->IP++;
    }

    return obj->fetched;
}