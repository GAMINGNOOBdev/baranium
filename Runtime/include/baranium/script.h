#ifndef __BARANIUM__SCRIPT_H_
#define __BARANIUM__SCRIPT_H_ 1

#include <stdint.h>
#include "defines.h"
#include "field.h"
#include "variable.h"
#include "function.h"

#define MAGIC_NUM_0 'B'
#define MAGIC_NUM_1 'G'
#define MAGIC_NUM_2 'S'
#define MAGIC_NUM_3 'L'

enum BaraniumSectionType
{
    BaraniumSectionType_Invalid,
    BaraniumSectionType_Fields,
    BaraniumSectionType_Variables,
    BaraniumSectionType_Functions,
};

typedef struct BaraniumSection
{
    struct BaraniumSection* prev;

    uint8_t Type;
    index_t ID;
    uint64_t DataSize;
    uint8_t* Data;

    struct BaraniumSection* next;
} BaraniumSection;

typedef struct BaraniumScriptHeader
{
    uint8_t MagicNumber[4];
    uint32_t Version;
    uint64_t SectionCount;
} BaraniumScriptHeader;

typedef struct BaraniumScriptNameTableEntry
{
    struct BaraniumScriptNameTableEntry* prev;

    uint8_t NameLength;
    uint8_t* Name;
    index_t ID;

    struct BaraniumScriptNameTableEntry* next;
} BaraniumScriptNameTableEntry;

typedef struct BaraniumScriptNameTable
{
    uint64_t NameCount;
    BaraniumScriptNameTableEntry* EntriesStart;
    BaraniumScriptNameTableEntry* EntriesEnd;
} BaraniumScriptNameTable;

typedef struct BaraniumScript
{
    BaraniumScriptHeader Header;
    BaraniumSection* SectionsStart;
    BaraniumSection* SectionsEnd;
    BaraniumScriptNameTable NameTable;
} BaraniumScript;

/**
 * @brief Open a script from a loaded file
 * 
 * @param handle The file handle containing the script data
 * @returns An instance of the loaded script, `null` if loading failed
 */
BARANIUMAPI BaraniumScript* baranium_open_script(BaraniumHandle* handle);

/**
 * @brief Safely close & dispose a script and it's data
 * 
 * @param script The script that will be disposed
 */
BARANIUMAPI void baranium_close_script(BaraniumScript* script);

/**
 * @brief Get the location/index of a specific item with requested `name`
 * 
 * @param script The script that contains the requested item
 * @param name The name of the item
 * 
 * @returns An index to that specific item
 */
BARANIUMAPI index_t baranium_script_get_location_of(BaraniumScript* script, const char* name);

#endif