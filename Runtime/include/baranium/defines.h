#ifndef __BARANIUM__DEFINES_H_
#define __BARANIUM__DEFINES_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/version.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

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

/////////////
/// Types ///
/////////////

typedef union baranium_value_t
{
    float numfloat;
    double numdouble;

    uint8_t num8;
    uint16_t num16;
    uint32_t num32;
    uint64_t num64;

    int8_t snum8;
    int16_t snum16;
    int32_t snum32;
    int64_t snum64;

    void* ptr;
} baranium_value_t;

typedef struct baranium_handle
{
    struct baranium_handle* prev;
    FILE* file;
    char* path;
    struct baranium_handle* next;
} baranium_handle;

// forward declarations because we need them
struct bstack;
struct bcpu;
struct bvarmgr;
struct baranium_library;
struct baranium_string_list;
struct baranium_callback_list;
struct baranium_function_manager;

typedef struct
{
    baranium_handle* start;
    baranium_handle* end;
    uint64_t open_handles;
    struct baranium_function_manager* function_manager;
    struct baranium_callback_list* callbacks;
    struct bstack* function_stack;
    struct bvarmgr* varmgr;
    struct bcpu* cpu;

    struct baranium_string_list* library_dir_contents;
    struct baranium_library** libraries;
    size_t library_buffer_size;
    const char* library_path;
    size_t library_count;
} baranium_runtime;


typedef int64_t index_t;
#define BARANIUM_INVALID_INDEX ((index_t)-1)

#ifdef __cplusplus
}
#endif

#endif
