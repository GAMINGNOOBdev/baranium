#include "CompiledScript.h"
#include "../Logging.h"
#include <algorithm>

namespace Binaries
{

    CompiledScript::CompiledScript()
        : mHeader(), mSections(), mCompiler(*this)
    {
        mHeader.MagicNumber[0] = MAGIC_NUM_0;
        mHeader.MagicNumber[1] = MAGIC_NUM_1;
        mHeader.MagicNumber[2] = MAGIC_NUM_2;
        mHeader.MagicNumber[3] = MAGIC_NUM_3;
        mHeader.Version = VERSION_CURRENT;
    }

    void CompiledScript::ParseTokens(TokenList& tokens)
    {
        mSections.clear();

        for (auto& token : tokens)
        {
            mLookupTable.AddEntry(token->mName, token->ID);

            switch (token->mTokenType)
            {
                case Language::TokenType::Field:
                {
                    std::shared_ptr<Language::Field> tokenObject = std::static_pointer_cast<Language::Field, Language::Token>(token);
                    CreateFieldSection(tokenObject);
                    break;
                }

                case Language::TokenType::Function:
                {
                    std::shared_ptr<Language::Function> tokenObject = std::static_pointer_cast<Language::Function, Language::Token>(token);
                    CreateFunctionSection(tokenObject);
                    break;
                }

                case Language::TokenType::Variable:
                {
                    std::shared_ptr<Language::Variable> tokenObject = std::static_pointer_cast<Language::Variable, Language::Token>(token);
                    CreateVariableSection(tokenObject);
                    break;
                }

                default:
                case Language::TokenType::Invalid:
                    Logging::Log("Invalid global token", Logging::Error);
                    Logging::Dispose();
                    exit(-1);
                    break;
            }
        }
    }

    void CompiledScript::Save(BgeFile& file)
    {
        if (!file.Ready())
            return;

        mHeader.SectionCount = mSections.size();

        file.Write(mHeader.MagicNumber, sizeof(uint8_t), 4);
        file.Write<uint32_t>(mHeader.Version);
        file.Write<uint64_t>(mHeader.SectionCount);

        for (auto& section : mSections)
        {
            file.Write<uint8_t>((uint8_t)section.Type);
            file.Write<index_t>(section.ID);
            file.Write<uint64_t>(section.DataSize);
            file.Write(section.Data, sizeof(uint8_t), section.DataSize);

            // there will always be memory allocated by sections
            if (section.Data != nullptr)
                free(section.Data);
        }

        mLookupTable.Write(file);

        // clear up memory
        mCompiler.ClearCompiledCode();
    }

    const NameLookupTable* CompiledScript::GetNameLookupTable()
    {
        return &mLookupTable;
    }

    const Section* CompiledScript::GetSection(index_t id, SectionType type)
    {
        if (id == -1)
            return nullptr;

        auto iterator = std::find_if(mSections.begin(), mSections.end(), [id,type](Section& section){
            return section.ID == id && section.Type == type;
        });

        if (iterator == mSections.end())
            return nullptr;

        return &(mSections.data()[iterator - mSections.begin()]);
    }

    const Section* CompiledScript::GetSection(std::string name, SectionType type)
    {
        return GetSection(mLookupTable.Lookup(name), type);
    }

    void CompiledScript::CreateFieldSection(std::shared_ptr<Language::Field> field)
    {
        Section fieldSection = Section();
        fieldSection.ID = field->ID;
        fieldSection.Type = SectionType::Field;

        // Size calculation: data type, data size
        uint8_t dataTypeSize = Language::VariableTypeBytes(field->Type);
        if (dataTypeSize == -1) // meaning this is a string
            dataTypeSize = field->Value.length() + 1; // for now store the initial string's length + 1 because of the null-char at the end

        size_t dataSize = sizeof(uint8_t) + dataTypeSize;
        fieldSection.DataSize = dataSize;
        fieldSection.Data = (uint8_t*)malloc(dataSize);
        memset(fieldSection.Data, 0, dataSize);

        mCompiler.CopyVariableData(fieldSection.Data, field->Value, field->Type);

        mSections.push_back(Section(fieldSection));
    }

    void CompiledScript::CreateVariableSection(std::shared_ptr<Language::Variable> variable)
    {
        Section variableSection = Section();
        variableSection.ID = variable->ID;
        variableSection.Type = SectionType::Variable;

        int8_t dataTypeSize = Language::VariableTypeBytes(variable->Type);
        if (dataTypeSize == -1) // meaning this is a string
            dataTypeSize = variable->Value.length() + 1; // for now store the initial string's length + 1 because of the null-char at the end

        // Size calculation: data type (1 byte) + data size
        size_t dataSize = sizeof(uint8_t) + dataTypeSize;
        variableSection.DataSize = dataSize;
        variableSection.Data = (uint8_t*)malloc(dataSize);
        memset(variableSection.Data, 0, dataSize);

        mCompiler.CopyVariableData(variableSection.Data, variable->Value, variable->Type);

        mSections.push_back(Section(variableSection));
    }

    void CompiledScript::CreateFunctionSection(std::shared_ptr<Language::Function> function)
    {
        Section functionSection = Section();
        functionSection.ID = function->ID;
        functionSection.Type = SectionType::Function;

        // "compile" the code
        mCompiler.ClearCompiledCode();
        mCompiler.Compile(function->mTokens);
        mCompiler.FinalizeCompilation();

        // Size calculation: compiled code size
        functionSection.DataSize = mCompiler.GetCompiledCodeSize();
        functionSection.Data = (uint8_t*)malloc(functionSection.DataSize);
        memcpy(functionSection.Data, mCompiler.GetCompiledCode(), functionSection.DataSize);

        // prepare for the next compilation + clear up duplicated memory
        mCompiler.ClearCompiledCode();

        mSections.push_back(Section(functionSection));
    }

}