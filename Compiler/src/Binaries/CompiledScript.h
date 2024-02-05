#ifndef __COMPILEDSCRIPT_H_
#define __COMPILEDSCRIPT_H_ 1

#include <BgeFile.hpp>
#include <stdint.h>
#include <vector>

#define MAGIC_NUM_0 'B'
#define MAGIC_NUM_1 'G'
#define MAGIC_NUM_2 'S'
#define MAGIC_NUM_3 'L'

namespace Binaries
{

    enum class SectionType : uint8_t
    {
        Invalid = -1,
        Fields,
        Variables,
        Functions,
    };

    struct Section
    {
        SectionType Type;
        uint64_t DataSize;
        uint64_t DataStart;

        // won't hold anything unless read/written by the user
        void* Data;
    };

    struct CompiledScriptHeader
    {
        uint8_t MagicNumber[4];
        uint16_t VersionHigh;
        uint8_t VersionMid;
        uint8_t VersionLow;
        uint64_t SectionCount;
    };

    struct CompiledScript
    {
        CompiledScriptHeader Header;
        std::vector<Section> Sections;

        /**
         * @brief Construct a new `CompiledScript` object
         */
        CompiledScript();

        /**
         * @brief Load a compiled script from
         * 
         * @param file The path to the compiled file
         */
        void Load(BgeFile& file);

        /**
         * @brief Write a compiled script
         * 
         * @param file The destination path to the compiled file
         */
        void Save(BgeFile& file);
    };

}

#endif