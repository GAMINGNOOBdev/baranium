#ifndef __BARANIUM__BBUS_H_
#define __BARANIUM__BBUS_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "../function.h"
#include <stdint.h>

typedef struct bbus
{
    baranium_function* dataHolder;
} bbus;

// create and initialize a bus
bbus* bbus_init(baranium_function* function);

// dispose of a bus
void bbus_dispose(bbus* obj);

// read from a bus
uint8_t bbus_read(bbus* obj, uint64_t addr);

// write to a bus
void bbus_write(bbus* obj, uint64_t addr, uint8_t value);

// check if the end of the bus has been reached
uint8_t bbus_eof(bbus* obj, uint64_t addr);

#ifdef __cplusplus
}
#endif

#endif
