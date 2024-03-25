#ifndef __BARANIUM__SCRIPT_H_
#define __BARANIUM__SCRIPT_H_ 1

#include <stdint.h>

#define MAGIC_NUM_0 'B'
#define MAGIC_NUM_1 'G'
#define MAGIC_NUM_2 'S'
#define MAGIC_NUM_3 'L'

#define VERSION_CREATE(year, month, day) ((year << 16) | (month << 8) | day)

#define VERSION_FIRST VERSION_CREATE(2024, 2, 10)
#define VERSION_CURRENT VERSION_FIRST

enum BaraniumSectionType
{
    Invalid,
    Fields,
    Variables,
    Functions,
};

typedef struct BaraniumSection
{
    enum BaraniumSectionType Type;
    uint64_t DataSize;
    uint64_t DataStart;
} BaraniumSection;

typedef struct BaraniumScriptHeader
{
    uint8_t MagicNumber[4];
    uint32_t Version;
    uint64_t SectionCount;
} BaraniumScriptHeader;

#endif