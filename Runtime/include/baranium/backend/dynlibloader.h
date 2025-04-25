/**
 * @note THIS IS NOT INTENDED FOR USE BY THE USER OF THE RUNTIME!
 *       This header is intended to be used internally by the runtime
 *       and therefore, functions defined in this header cannot be used
 *       by the user.
 */
#ifndef __BARANIUM__BACKEND__DYNLIBLOADER_H_
#define __BARANIUM__BACKEND__DYNLIBLOADER_H_ 1

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
#   define BARANIUM_DYNLIB_EXTENSION ".dll"
#else
#   define BARANIUM_DYNLIB_EXTENSION ".so"
#endif

#ifdef BARANIUM_DYNLIB_EXPORT
#   define BARANIUM_DYNLIB_INIT() \
    baranium_dynlib_data dynlib_init_data = {NULL,NULL,NULL,NULL};\
    baranium_runtime* baranium_get_runtime_dynlib(void)\
    {\
        if (dynlib_init_data.runtime == NULL) return NULL; \
        return (baranium_runtime*)dynlib_init_data.runtime;\
    }\
    size_t baranium_variable_get_size_of_type_dynlib(baranium_variable_type_t type)\
    {\
        if (dynlib_init_data.vargetsize == NULL) return -1; \
        size_t(*func)(baranium_variable_type_t); \
        func = (size_t(*)(baranium_variable_type_t))dynlib_init_data.vargetsize; \
        return func(type);\
    }\
    void baranium_compiled_variable_push_to_stack_dynlib(bcpu* cpu, baranium_compiled_variable* var)\
    {\
        if (dynlib_init_data.compvarpushtostack == NULL) return;\
        void(*func)(bcpu*, baranium_compiled_variable*); \
        func = (void(*)(bcpu*, baranium_compiled_variable*))dynlib_init_data.compvarpushtostack;\
        func(cpu, var);\
    }\
    void baranium_compiled_variable_convert_to_type_dynlib(baranium_compiled_variable* var, baranium_variable_type_t targetType)\
    {\
        if (dynlib_init_data.compvarconverttotype == NULL) return;\
        void(*func)(baranium_compiled_variable*, baranium_variable_type_t); \
        func = (void(*)(baranium_compiled_variable*, baranium_variable_type_t))dynlib_init_data.compvarconverttotype; \
        func(var, targetType);\
    }\
    void __dynlib_initialize__(baranium_dynlib_data* data) { dynlib_init_data = *data; }
#else
#   define BARANIUM_DYNLIB_INIT()
#endif

typedef struct
{
    void* runtime;
    void* vargetsize; // baranium_variable_get_size_of_type function pointer
    void* compvarpushtostack;// baranium_compiled_variable_push_to_stack function pointer
    void* compvarconverttotype;// baranium_compiled_variable_convert_to_type function pointer
} baranium_dynlib_data;

typedef void* baranium_dynlib_handle;

/**
 * @brief Load a dynamic library (automatically calls the "init" function with the given data)
 * 
 * @param filepath Dynamic library file path
 * @param data Initialization data
 * @returns A dynamic library handle
 */
baranium_dynlib_handle baranium_dynlib_load(const char* filepath, baranium_dynlib_data* data);

/**
 * @brief Get a symbol (aka function) from a dynamic library
 * 
 * @param handle Dynamic library handle
 * @param name Symbol name
 * @returns The loaded symbol's address
 */
void* baranium_dynlib_symbol(baranium_dynlib_handle handle, const char* name);

/**
 * @brief Unload a dynamic library
 * 
 * @param handle Dynamic library handle
 */
void baranium_dynlib_unload(baranium_dynlib_handle handle);

#endif
