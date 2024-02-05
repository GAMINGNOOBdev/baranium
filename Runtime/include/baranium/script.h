#ifndef __BARANIUM__SCRIPT_H_
#define __BARANIUM__SCRIPT_H_ 1

#include <stdint.h>

#define MAGIC_NUM_0 'B'
#define MAGIC_NUM_1 'G'
#define MAGIC_NUM_2 'S'
#define MAGIC_NUM_3 'L'

enum BaraniumSectionType
{
    Invalid = -1,
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
    uint16_t VersionHigh;
    uint8_t VersionMid;
    uint8_t VersionLow;
    uint64_t SectionCount;
} BaraniumScriptHeader;

#endif