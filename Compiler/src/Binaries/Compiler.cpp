#include "../Logging.h"
#include "Compiler.h"
#include <memory.h>

namespace Binaries
{

    Compiler::Compiler()
        : mCode(nullptr), mCodeLength(0), mCodeBuilder()
    {
    }

    void Compiler::ClearCompiledCode()
    {
        if (mCode != nullptr)
            free(mCode);
        
        mCode = nullptr;
        mCodeLength = 0;
        mCodeBuilder.clear();
    }

    uint8_t* Compiler::GetCompiledCode()
    {
        return mCode;
    }

    size_t Compiler::GetCompiledCodeSize()
    {
        return mCodeLength;
    }

    void Compiler::CopyVariableData(void* __dst, std::string __src, Language::VariableType varType)
    {
        (*(uint8_t*)__dst) = (uint8_t)varType;
        void* dest = (void*)((uint64_t)__dst + 1);

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
                memcpy(dest, &objID, sizeof(int64_t));
                break;
            }

            case Language::VariableType::String:
            {
                memcpy(dest, __src.data(), __src.length());
                break;
            }

            case Language::VariableType::Float:
            {
                float fValue;
                if (!__src.empty())
                    fValue = std::stof(__src);
                memcpy(dest, &fValue, sizeof(float));
                break;
            }

            case Language::VariableType::Bool:
            {
                uint8_t bValue = 0;
                if (!__src.empty())
                    bValue = __src == "true" ? 1 : 0;
                memcpy(dest, &bValue, sizeof(uint8_t));
                break;
            }

            case Language::VariableType::Int:
            {
                int32_t iValue = 0;
                if (!__src.empty())
                    iValue = std::stoi(__src);
                memcpy(dest, &iValue, sizeof(int32_t));
                break;
            }

            case Language::VariableType::Uint:
            {
                uint32_t uValue = 0;
                if (!__src.empty())
                    uValue = (uint32_t)std::stoi(__src);
                memcpy(dest, &uValue, sizeof(uint32_t));
                break;
            }

        }
    }

    void Compiler::Compile(TokenList& tokens)
    {
        for (size_t i = 0; i < tokens.size(); i++)
        {
            auto& token = tokens.at(i);

            if (token->mTokenType == Language::TokenType::Function)
                Logging::LogErrorExit("Trying to compile function inside function, bruh");

            if (token->mTokenType == Language::TokenType::Field)
                Logging::LogErrorExit("Fields should not be inside functions my man");

            if (token->mTokenType == Language::TokenType::Variable)
                CompileVariable(std::static_pointer_cast<Language::Variable>(token));

            if (token->mTokenType == Language::TokenType::Expression)
                CompileExpression(std::static_pointer_cast<Language::Expression>(token));

            if (token->mTokenType == Language::TokenType::IfElseStatement)
                CompileIfElseStatement(std::static_pointer_cast<Language::IfElseStatement>(token));

            if (token->mTokenType == Language::TokenType::DoWhileLoop)
                CompileDoWhileLoop(std::static_pointer_cast<Language::Loop>(token));

            if (token->mTokenType == Language::TokenType::WhileLoop)
                CompileWhileLoop(std::static_pointer_cast<Language::Loop>(token));

            if (token->mTokenType == Language::TokenType::ForLoop)
                CompileForLoop(std::static_pointer_cast<Language::Loop>(token));
        }
    }

    void Compiler::FinalizeCompilation()
    {
        if (mCode != nullptr)
            free(mCode);

        mCodeLength = mCodeBuilder.size();
        mCode = (uint8_t*)malloc(sizeof(uint8_t)*mCodeLength);
        memcpy(mCode, mCodeBuilder.data(), mCodeLength);
    }

    void Compiler::CompileVariable(std::shared_ptr<Language::Variable> token)
    {
        /// TODO: --- implement ---
    }

    void Compiler::CompileExpression(std::shared_ptr<Language::Expression> token)
    {
        /// TODO: --- implement ---
    }

    void Compiler::CompileIfElseStatement(std::shared_ptr<Language::IfElseStatement> token)
    {
        /// TODO: --- implement ---
    }

    void Compiler::CompileDoWhileLoop(std::shared_ptr<Language::Loop> token)
    {
        /// TODO: --- implement ---
    }

    void Compiler::CompileWhileLoop(std::shared_ptr<Language::Loop> token)
    {
        /// TODO: --- implement ---
    }

    void Compiler::CompileForLoop(std::shared_ptr<Language::Loop> token)
    {
        /// TODO: --- implement ---
    }


}