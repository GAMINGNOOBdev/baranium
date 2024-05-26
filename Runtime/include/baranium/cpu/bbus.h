#ifndef __BARANIUM__BBUS_H_
#define __BARANIUM__BBUS_H_ 1

#include "../function.h"
#include <stdint.h>

typedef struct bbus
{
    BaraniumFunction* dataHolder;
} bbus;

void bbus_init(bbus* obj, BaraniumFunction* function);
uint8_t bbus_read(bbus* obj, uint64_t addr);
void bbus_write(bbus* obj, uint64_t addr, uint8_t value);
uint8_t bbus_eof(bbus* obj, uint64_t addr);

#endif