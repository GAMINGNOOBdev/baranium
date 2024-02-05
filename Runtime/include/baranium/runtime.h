#ifndef __BARANIUM__RUNTIME_H_
#define __BARANIUM__RUNTIME_H_ 1

#include "defines.h"

BARANIUMAPI BaraniumRuntime* baranium_init();
BARANIUMAPI void baranium_set_context(BaraniumRuntime* runtimeContext);
BARANIUMAPI void baranium_cleanup(BaraniumRuntime* runtime);
BARANIUMAPI BaraniumHandle* baranium_open_handle(const char* source);
BARANIUMAPI void baranium_close_handle(BaraniumHandle* handle);

#endif