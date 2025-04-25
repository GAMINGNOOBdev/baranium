#include <baranium/backend/dynlibloader.h>
#include <baranium/logging.h>
#include <baranium/defines.h>

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_UNDEFINED
#   error "Invalid build platform"
#elif BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
#   include <Windows.h>
#   define libopen(name)    LoadLibraryA(name)
#   define libsym           GetProcAddress
#   define libclose         FreeLibrary
#elif BARANIUM_PLATFORM == BARANIUM_PLATFORM_PSP
#   define libopen(name)        NULL
#   define libsym(handle,sym)   NULL
#   define libclose(handle)
#else
#   include <dlfcn.h>
#   define libopen(name)    dlopen(name, RTLD_LAZY)
#   define libsym           dlsym
#   define libclose         dlclose
#endif

baranium_dynlib_handle baranium_dynlib_load(const char* filepath, baranium_dynlib_data* data)
{
    baranium_dynlib_handle handle = libopen(filepath);
    LOGINFO(stringf("loaded library 0x%x", handle));

    void(*initsymbol)(baranium_dynlib_data*) = (void(*)(baranium_dynlib_data*))baranium_dynlib_symbol(handle, "__dynlib_initialize__");
    if (initsymbol != NULL)
        initsymbol(data);

    return handle;
}

void* baranium_dynlib_symbol(baranium_dynlib_handle handle, const char* name)
{
    void* sym = libsym(handle, name);
    LOGINFO(stringf("loaded symbol '%s' (0x%x) from library 0x%x", name, sym, handle));
    return sym;
}

void baranium_dynlib_unload(baranium_dynlib_handle handle)
{
    LOGINFO(stringf("unloading library with address 0x%x", handle));
    libclose(handle);
}
