#include <baranium/cpu/bbus.h>
#include <baranium/logging.h>
#include <stdlib.h>
#include <memory.h>

bbus* bbus_init(baranium_function* function)
{
    bbus* obj = malloc(sizeof(bbus));
    if (obj == NULL) return NULL;

    memset(obj, 0, sizeof(bbus));
    obj->data_holder = function;

    return obj;
}

void bbus_dispose(bbus* obj)
{
    if (!obj) return;

    free(obj);
}

uint8_t bbus_read(struct bbus* obj, uint64_t addr)
{
    if (obj == NULL) return 0;
    if (!obj->data_holder) return 0;
    if (addr >= obj->data_holder->data_size) return 0;

    uint8_t* data = obj->data_holder->data;
    return data[addr];
}

// this shouldn't do anything, since we could badly damage and corrupt code
// keep commented out until needed one day maybe
// void bbus_write(bbus* obj, uint64_t addr, uint8_t value)
// {
//     if (obj == NULL || addr == (uint64_t)-1)
//         return;

//     uint8_t* data = obj->dataHolder->Data;
//     data[addr] = value;
// }

uint8_t bbus_eof(bbus* obj, uint64_t addr)
{
    if (obj == NULL)
        return 1;

    if (obj->data_holder == NULL)
        return 1;

    return addr >= obj->data_holder->data_size;
}
