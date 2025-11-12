/**
 * @note THIS IS NOT INTENDED FOR USE BY THE USER OF THE RUNTIME!
 *       This header is intended to be used internally by the runtime
 *       and therefore, functions defined in this header cannot be used
 *       by the user.
 */
#ifndef __BARANIUM__BACKEND__BVARMGR_H_
#define __BARANIUM__BACKEND__BVARMGR_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "../variable.h"
#include "../field.h"

typedef struct bvarmgr_n
{
    struct bvarmgr_n* prev;
    baranium_variable* variable;
    baranium_field* field;
    uint8_t isVariable;
    struct bvarmgr_n* next;
} bvarmgr_n;

typedef struct bvarmgr
{
    bvarmgr_n* start;
    bvarmgr_n* end;
    size_t count;
} bvarmgr;

// create and initialize a variable manager
bvarmgr* bvarmgr_init(void);

// dispose a variable manager
void bvarmgr_dispose(bvarmgr* obj);

// clear a variable manager
void bvarmgr_clear(bvarmgr* obj);

// allocate/create a variable
void bvarmgr_alloc(bvarmgr* obj, baranium_variable_type_t type, index_t id, size_t size, uint8_t isField);

// get a created entry if existent
bvarmgr_n* bvarmgr_get(bvarmgr* obj, index_t id);

// delete and free memory used by a variable
void bvarmgr_dealloc(bvarmgr* obj, index_t id);

#ifdef __cplusplus
}
#endif

#endif
