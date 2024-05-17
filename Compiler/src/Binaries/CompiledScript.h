#ifndef __BINARIES__COMPILEDSCRIPT_H_
#define __BINARIES__COMPILEDSCRIPT_H_ 1

#include "../Language/Types.h"
#include "NameLookupTable.h"
#include <BgeFile.hpp>
#include "Compiler.h"
#include "Section.h"
#include <stdint.h>
#include <vector>

#define MAGIC_NUM_0 'B'
#define MAGIC_NUM_1 'G'
#define MAGIC_NUM_2 'S'
#define MAGIC_NUM_3 'L'

#define VERSION_CREATE(year, month, day) ((year << 16) | (month << 8) | day)

#define VERSION_FIRST VERSION_CREATE(2024, 2, 10)
#define VERSION_CURRENT VERSION_FIRST

typedef int64_t index_t;

namespace Binaries
{

    struct CompiledScriptHeader
    {
        uint8_t MagicNumber[4];
        uint32_t Version;
        uint64_t SectionCount;
    };

    struct CompiledScript
    {
        /**
         * @brief Construct a new `CompiledScript` object
         */
        CompiledScript();

        /**
         * @brief Parse a list of tokens into sections
         * 
         * @param tokens The source code tokens
         */
        void ParseTokens(TokenList& tokens);

        /**
         * @brief Write a compiled script
         * 
         * @param file The destination path to the compiled file
         */
        void Save(BgeFile& file);

        /**
         * @brief Get the name lookup table
         */
        const NameLookupTable* GetNameLookupTable();

        /**
         * @brief Get a  section by id
         */
        const Section* GetSection(index_t id, SectionType type);

        /**
         * @brief Get a  section by name
         */
        const Section* GetSection(std::string name, SectionType type);

    private:
        CompiledScriptHeader mHeader;
        std::vector<Section> mSections;
        NameLookupTable mLookupTable;
        Compiler mCompiler;

    private:
        void CreateFieldSection(std::shared_ptr<Language::Field> field);
        void CreateVariableSection(std::shared_ptr<Language::Variable> variable);
        void CreateFunctionSection(std::shared_ptr<Language::Function> function);
    };

}

#endif