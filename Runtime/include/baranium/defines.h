#ifndef __BARANIUM__DEFINES_H_
#define __BARANIUM__DEFINES_H_ 1

#include <stdint.h>
#include <stdio.h>
#include "bcpu.h"

#define BARANIUM_PLATFORM_UNDEFINED  0
#define BARANIUM_PLATFORM_WINDOWS    1
#define BARANIUM_PLATFORM_LINUX      2
#define BARANIUM_PLATFORM_APPLE      3
#define BARANIUM_PLATFORM_SWITCH     4
#define BARANIUM_PLATFORM_PSP        6

#define BARANIUM_PLATFORM_NAME_UNKNOWN   "Unknown"
#define BARANIUM_PLATFORM_NAME_WINDOWS   "Windows"
#define BARANIUM_PLATFORM_NAME_LINUX     "Linux"
#define BARANIUM_PLATFORM_NAME_APPLE     "Apple MacOS"
#define BARANIUM_PLATFORM_NAME_SWITCH    "Nintendo Switch"
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
#elif defined(__SWITCH__)
#   define BARANIUM_PLATFORM         BARANIUM_PLATFORM_SWITCH
#   define BARANIUM_PLATFORM_NAME    BARANIUM_PLATFORM_NAME_SWITCH
#elif defined(__psp__)
#   define BARANIUM_PLATFORM         BARANIUM_PLATFORM_PSP
#   define BARANIUM_PLATFORM_NAME    BARANIUM_PLATFORM_NAME_PSP
#else
#   define BARANIUM_PLATFORM         BARANIUM_PLATFORM_UNDEFINED
#   define BARANIUM_PLATFORM_NAME    BARANIUM_PLATFORM_NAME_UNKNOWN
#   error Could not detect your operating system
#endif

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
#   ifdef BARANIUM_BUILD
#       define BARANIUMAPI __declspec(dllexport)
#   else
#       define BARANIUMAPI __declspec(dllimport)
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

typedef struct BaraniumHandle
{
    FILE* file;
    struct BaraniumHandle* prev;
    struct BaraniumHandle* next;
} BaraniumHandle;

typedef struct BaraniumRuntime
{
    BaraniumHandle* start;
    BaraniumHandle* end;
    uint64_t openHandles;
    BCpu cpu;
} BaraniumRuntime;


typedef int64_t index_t;
#define INVALID_INDEX ((index_t)-1)

#endif