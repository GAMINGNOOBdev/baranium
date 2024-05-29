#ifndef __BARANIUM__BBUS_H_
#define __BARANIUM__BBUS_H_ 1

#include "../function.h"
#include <stdint.h>

typedef struct bbus
{
    BaraniumFunction* dataHolder;
} bbus;

// create and initialize a bus
bbus* bbus_init(BaraniumFunction* function);

// dispose of a bus
void bbus_dispose(bbus* obj);

// read from a bus
uint8_t bbus_read(bbus* obj, uint64_t addr);

// write to a bus
void bbus_write(bbus* obj, uint64_t addr, uint8_t value);

// check if the end of the bus has been reached
uint8_t bbus_eof(bbus* obj, uint64_t addr);

#endif