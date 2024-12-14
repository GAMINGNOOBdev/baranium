#ifndef __BARANIUM__DEFINES_H_
#define __BARANIUM__DEFINES_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define BARANIUM_VERSION_MAJOR       0
#define BARANIUM_VERSION_MINOR       1
#define BARANIUM_VERSION_PHASE       "pre-alpha"

#define BARANIUM_PLATFORM_UNDEFINED  0
#define BARANIUM_PLATFORM_WINDOWS    1
#define BARANIUM_PLATFORM_LINUX      2
#define BARANIUM_PLATFORM_APPLE      3
#define BARANIUM_PLATFORM_PSP        4

#define BARANIUM_PLATFORM_NAME_UNKNOWN   "Unknown"
#define BARANIUM_PLATFORM_NAME_WINDOWS   "Windows"
#define BARANIUM_PLATFORM_NAME_LINUX     "Linux"
#define BARANIUM_PLATFORM_NAME_APPLE     "Apple MacOS"
#define BARANIUM_PLATFORM_NAME_PSP       "PlayStationPortable (PSP)"

#if defined(_WIN32) || defined(WINAPI_FAMILY)
#   define BARANIUM_PLATFORM         BARANIUM_PLATFORM_WINDOWS
#   define BARANIUM_PLATFORM_NAME    BARANIUM_PLATFORM_NAME_WINDOWS
#elif defined(__linux__)
#   define BARANIUM_PLATFORM         BARANIUM_PLATFORM_LINUX
#   define BARANIUM_PLATFORM_NAME    BARANIUM_PLATFORM_NAME_LINUX
#elif defined(__APPLE__)
#   define BARANIUM_PLATFORM         BARANIUM_PLATFORM_APPLE
#   define BARANIUM_PLATFORM_NAME    BARANIUM_PLATFORM_NAME_APPLE
#elif defined(__psp__)
#   define BARANIUM_PLATFORM         BARANIUM_PLATFORM_PSP
#   define BARANIUM_PLATFORM_NAME    BARANIUM_PLATFORM_NAME_PSP
#else
#   define BARANIUM_PLATFORM         BARANIUM_PLATFORM_UNDEFINED
#   define BARANIUM_PLATFORM_NAME    BARANIUM_PLATFORM_NAME_UNKNOWN
#   error Could not detect your operating system
#endif

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
#   ifdef BARANIUM_DYNAMIC
#       ifdef BARANIUM_BUILD
#           define BARANIUMAPI __declspec(dllexport)
#       else
#           define BARANIUMAPI __declspec(dllimport)
#       endif
#   else
#       define BARANIUMAPI
#   endif
#else
#   define BARANIUMAPI
#endif

#define VERSION_CREATE(year, month, day) ((year << 16) | (month << 8) | day)

#define VERSION_FIRST VERSION_CREATE(2024, 2, 10)
#define VERSION_CURRENT VERSION_FIRST

/////////////
/// Types ///
/////////////

typedef struct baranium_handle
{
    struct baranium_handle* prev;
    FILE* file;
    struct baranium_handle* next;
} baranium_handle;

// forward declarations because we need them
struct bstack;
struct bcpu;
struct bvarmgr;
struct baranium_function_manager;

typedef struct
{
    baranium_handle* start;
    baranium_handle* end;
    uint64_t openHandles;
    struct baranium_function_manager* functionManager;
    struct bstack* functionStack;
    struct bvarmgr* varmgr;
    struct bcpu* cpu;
} baranium_runtime;


typedef int64_t index_t;
#define INVALID_INDEX ((index_t)-1)

#ifdef __cplusplus
}
#endif

#endif
