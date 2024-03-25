#include "CompiledScript.h"
#include "../Logging.h"

namespace Binaries
{

    /**
     * @brief Construct a new `CompiledScript` object
     */
    CompiledScript::CompiledScript()
        : mHeader(), mSections(), mCurrentOffset(0)
    {
        mHeader.MagicNumber[0] = MAGIC_NUM_0;
        mHeader.MagicNumber[1] = MAGIC_NUM_1;
        mHeader.MagicNumber[2] = MAGIC_NUM_2;
        mHeader.MagicNumber[3] = MAGIC_NUM_3;
        mHeader.Version = VERSION_CURRENT;
        mCurrentOffset = sizeof(CompiledScriptHeader);
    }

    /**
     * @brief Parse a list of tokens into sections
     * 
     * @param tokens The source code tokens
     */
    void CompiledScript::ParseTokens(TokenList& tokens)
    {
        mSections.clear();

        for (auto& token : tokens)
        {
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

    /**
     * @brief Write a compiled script
     * 
     * @param file The destination path to the compiled file
     */
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
            file.Write<uint64_t>(section.DataSize);
            file.Write<uint64_t>(section.DataStart);
        }
    }

    void CompiledScript::CreateFieldSection(std::shared_ptr<Language::Field> field)
    {
        Section fieldSection = Section();
        fieldSection.Type = SectionType::Field;
        // Size calculation: data type, initial value, name length, name
        fieldSection.DataSize = 1 + Language::VariableTypeBytes(field->Type) + 1 + field->mName.length();
        fieldSection.DataStart = -1;
        mSections.push_back(Section(fieldSection));
    }

    void CompiledScript::CreateVariableSection(std::shared_ptr<Language::Variable> variable)
    {
        Section fieldSection = Section();
        fieldSection.Type = SectionType::Variable;
        // Size calculation: data type, initial value, name length, name
        fieldSection.DataSize = 1 + Language::VariableTypeBytes(variable->Type) + 1 + variable->mName.length();
        fieldSection.DataStart = -1;
        mSections.push_back(Section(fieldSection));
    }

    void CompiledScript::CreateFunctionSection(std::shared_ptr<Language::Function> function)
    {
        Section fieldSection = Section();
        fieldSection.Type = SectionType::Function;
        // Size calculation: name length, name, internal tokens size
        fieldSection.DataSize = 1 + function->mName.length() + function->mTokens.size();
        fieldSection.DataStart = -1;
        mSections.push_back(Section(fieldSection));
    }

}