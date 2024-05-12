#include <baranium/cpu/bbus.h>
#include <baranium/logging.h>
#include <stdlib.h>
#include <memory.h>

uint8_t bbus_read(struct bbus* obj, uint64_t addr)
{
    if (obj == NULL)
        return 0;
    
    if (!obj->dataHolder)
        return 0;
    
    if (addr >= obj->dataHolder->DataSize)
        return 0;

    uint8_t* data = obj->dataHolder->Data;
    return data[addr];
}

// this shouldn't do anything, since we could badly damage and corrupt code
void bbus_write(struct bbus* obj, uint64_t addr, uint8_t value)
{
    return;
}

uint8_t bbus_eof(struct bbus* obj, uint64_t addr)
{
    if (obj == NULL)
        return 1;

    if (obj->dataHolder == NULL)
        return 1;

    return addr >= obj->dataHolder->DataSize;
}

void bbus_init(bbus* obj, BaraniumFunction* function)
{
    if (obj == NULL)
        return;

    obj->dataHolder = function;
    obj->read = bbus_read;
    obj->write = bbus_write;
    obj->eof = bbus_eof;
}