#include <baranium/cpu/bcpu_opcodes.h>
#include <baranium/logging.h>
#include <baranium/bcpu.h>
#include <stdlib.h>
#include <memory.h>

// function references (all declared at the end of this file)
uint64_t _bcpu_fetch(bcpu* obj, int bits);

// initializes the cpu with the requered function pointers and also sets the bus of the cpu
bcpu* bcpu_init()
{
    bcpu* obj = malloc(sizeof(bcpu));
    if (obj == NULL)
        return;

    memset(obj, 0, sizeof(bcpu));
    obj->fetched = 0;
    obj->fetch = _bcpu_fetch;
    obj->ticks = 0;
    obj->killTriggered = 0;
    bcpu_reset(obj);

    return obj;
}

void bcpu_dispose(bcpu* obj)
{
    if (obj == NULL)
        return;

    bstack_dispose(obj->stack);
    bstack_dispose(obj->ip_stack);
    bbus_dispose(obj->bus);
    bvarmgr_dispose(obj->varmgr);

    free(obj);
}

// this is the method which executes the instructions from the IP value forward
void bcpu_tick(bcpu* obj)
{
    if (obj == NULL)
        return;

    if (bbus_eof(obj->bus, obj->IP))
    {
        obj->killTriggered = 1;
        return;
    }

    obj->opcode = bbus_read(obj->bus, obj->IP);
    LOGDEBUG(stringf("IP: 0x%2.16x | Ticks (total): 0x%2.16x | Opcode: 0x%2.2x | Instruction: '%s'",
               obj->IP, obj->ticks, obj->opcode, opcodes[obj->opcode].name));
    obj->IP++;
    opcodes[obj->opcode].handle(obj);
    obj->ticks++;
}

// resets the cpu
void bcpu_reset(bcpu* obj)
{
    if (obj == NULL)
        return;

    obj->IP = 0;
    obj->stack = bstack_init();
    obj->ip_stack = bstack_init();
    obj->flags.CMP = 1;
    obj->flags.RESERVED = 0;
    obj->ticks = 0;
    obj->bus = bbus_init(NULL);
    obj->varmgr = bvarmgr_init();
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
    int bitindex = bits - 8;

    obj->fetched = 0;
    for (int i = 0; i < bytes; i++)
    {
        uint8_t data = bbus_read(obj->bus, obj->IP);
        obj->fetched |= data << bitindex;
        bitindex -= 8;
        obj->IP++;
    }

    return obj->fetched;
}