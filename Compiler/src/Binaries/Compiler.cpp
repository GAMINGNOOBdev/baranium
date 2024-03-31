#include "Compiler.h"
#include <memory.h>

namespace Binaries
{

    /**
     * @brief Construct a new `Compiler`
     */
    Compiler::Compiler()
        : mCode(nullptr), mCodeLength(0), mCodeBuilder()
    {
    }

    /**
     * @brief Clear the currently compiled code
     * 
     * @note This should be called after compiling a function and moving onto the next
     */
    void Compiler::ClearCompiledCode()
    {
        if (mCode != nullptr)
            free(mCode);
        
        mCode = nullptr;
        mCodeLength = 0;
        mCodeBuilder.clear();
    }

    /**
     * @brief Get the compiled code
     * 
     * @returns A pointer to the compiled code data
     */
    uint8_t* Compiler::GetCompiledCode()
    {
        return mCode;
    }

    /**
     * @brief Get the size in bytes of the compiled code
     * 
     * @returns The size of the compiled code
     */
    size_t Compiler::GetCompiledCodeSize()
    {
        return mCodeLength;
    }

    /**
     * @brief Small utility function of a compiler that copies the value of a variable into a destination buffer
     * 
     * @param __dst The destination buffer that will hold the data
     * @param __src The string representation of the data
     * @param varType The variable type
     */
    void Compiler::CopyVariableData(void* __dst, std::string __src, Language::VariableType varType)
    {
        switch (varType)
        {
            default:
            case Language::VariableType::Invalid:
            case Language::VariableType::Void:
                break;

            case Language::VariableType::GameObject:
            {
                int64_t objID;
                if (__src.empty())
                    objID = 0;
                else if (__src == Language::Keywords[KeywordIndex_null].Name)
                    objID = 0;
                else if (__src == Language::Keywords[KeywordIndex_attached].Name)
                    objID = -1;
                else
                    objID = std::stoll(__src);
                memcpy(__dst, &objID, sizeof(int64_t));
                break;
            }

            case Language::VariableType::String:
            {
                memcpy(__dst, __src.data(), __src.length());
                break;
            }

            case Language::VariableType::Float:
            {
                float fValue;
                if (!__src.empty())
                    fValue = std::stof(__src);
                memcpy(__dst, &fValue, sizeof(float));
                break;
            }

            case Language::VariableType::Bool:
            {
                uint8_t bValue = 0;
                if (!__src.empty())
                    bValue = __src == "true" ? 1 : 0;
                memcpy(__dst, &bValue, sizeof(uint8_t));
                break;
            }

            case Language::VariableType::Int:
            {
                int32_t iValue = 0;
                if (!__src.empty())
                    iValue = std::stoi(__src);
                memcpy(__dst, &iValue, sizeof(int32_t));
                break;
            }

            case Language::VariableType::Uint:
            {
                uint32_t uValue = 0;
                if (!__src.empty())
                    uValue = (uint32_t)std::stoi(__src);
                memcpy(__dst, &uValue, sizeof(uint32_t));
                break;
            }

        }
    }

    /**
     * @brief Compile the given tokens into binary
     * 
     * @param tokens The tokens that will be compiled
     */
    void Compiler::Compile(TokenList& tokens)
    {
        for (size_t i = 0; i < tokens.size(); i++)
            mCodeBuilder.push_back(tokens.size()-i);
    }

    /**
     * @brief Finalize the compilation of tokens
     */
    void Compiler::FinalizeCompilation()
    {
        if (mCode != nullptr)
            free(mCode);

        mCodeLength = mCodeBuilder.size();
        mCode = (uint8_t*)malloc(sizeof(uint8_t)*mCodeLength);
        memcpy(mCode, mCodeBuilder.data(), mCodeLength);
    }

}