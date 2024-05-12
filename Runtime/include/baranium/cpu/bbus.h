#ifndef __BARANIUM__BBUS_H_
#define __BARANIUM__BBUS_H_ 1

#include "../function.h"
#include <stdint.h>

struct bbus;
typedef uint8_t(*bbus_read_t)(struct bbus* obj, uint64_t addr);
typedef void(*bbus_write_t)(struct bbus* obj, uint64_t addr, uint8_t value);
typedef uint8_t(*bbus_eof_t)(struct bbus* obj, uint64_t addr);

typedef struct bbus
{
    bbus_read_t read;
    bbus_write_t write;
    bbus_eof_t eof;
    BaraniumFunction* dataHolder;
} bbus;

void bbus_init(bbus* obj, BaraniumFunction* function);

#endif