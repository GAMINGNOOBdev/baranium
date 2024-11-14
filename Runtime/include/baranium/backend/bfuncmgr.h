#ifndef __BARANIUM__BACKEND__BFUNCMGR_H_
#define __BARANIUM__BACKEND__BFUNCMGR_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "../function.h"
#include "../script.h"

typedef struct bfuncmgr_n
{
    struct bfuncmgr_n* prev;
    index_t id;
    BaraniumScript* script;
    struct bfuncmgr_n* next;
} bfuncmgr_n;

typedef struct baranium_function_manager
{
    bfuncmgr_n* start;
    bfuncmgr_n* end;
    size_t count;
} baranium_function_manager;

// create and initialize a function manager
baranium_function_manager* baranium_function_manager_init();

// dispose a function manager
void baranium_function_manager_dispose(baranium_function_manager* obj);

// clear a function manager
void baranium_function_manager_clear(baranium_function_manager* obj);

// allocate/create a function entry
void baranium_function_manager_add(baranium_function_manager* obj, index_t id, BaraniumScript* script);

// get a function if existent
BaraniumFunction* baranium_function_manager_get(baranium_function_manager* obj, index_t id);

// get a function entry if existent
bfuncmgr_n* baranium_function_manager_get_entry(baranium_function_manager* obj, index_t id);

// delete and free memory used by a function entry
void baranium_function_manager_remove(baranium_function_manager* obj, index_t id);

#ifdef __cplusplus
}
#endif

#endif