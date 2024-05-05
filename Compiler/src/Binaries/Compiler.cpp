#include "../Logging.h"
#include "Compiler.h"
#include <memory.h>

#define nop (void)0

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
        mCodeBuilder.Clear();
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

        mCodeLength = mCodeBuilder.Size();
        mCode = (uint8_t*)malloc(sizeof(uint8_t)*mCodeLength);
        memcpy(mCode, mCodeBuilder.Data(), mCodeLength);
    }

    void Compiler::CompileVariable(std::shared_ptr<Language::Variable> token)
    {token != nullptr ? CompileVariable(*token) : nop;}

    void Compiler::CompileVariable(Language::Variable& token)
    {
        mCodeBuilder.NOP();

        /// TODO: --- implement ---
    }

    void Compiler::CompileIfElseStatement(std::shared_ptr<Language::IfElseStatement> token)
    {token != nullptr ? CompileIfElseStatement(*token) : nop;}

    void Compiler::CompileIfElseStatement(Language::IfElseStatement& token)
    {
        CompileExpression(token.Condition);
        Compile(token.mTokens);

        for (auto& otherStatement : token.ChainedStatements)
            CompileIfElseStatement(otherStatement);
    }

    void Compiler::CompileDoWhileLoop(std::shared_ptr<Language::Loop> token)
    {token != nullptr ? CompileDoWhileLoop(*token) : nop;}

    void Compiler::CompileDoWhileLoop(Language::Loop& token)
    {
        uint16_t pointer = mCodeBuilder.Size() + 1;
        Compile(token.mTokens);
        CompileExpression(token.Condition);
        mCodeBuilder.JMP(pointer);
    }

    void Compiler::CompileWhileLoop(std::shared_ptr<Language::Loop> token)
    {token != nullptr ? CompileWhileLoop(*token) : nop;}

    void Compiler::CompileWhileLoop(Language::Loop& token)
    {
        /// TODO: --- implement better ---

        uint16_t pointer = mCodeBuilder.Size() + 1;
        CompileExpression(token.Condition);
        Compile(token.mTokens);
        mCodeBuilder.JMP(pointer);
    }

    void Compiler::CompileForLoop(std::shared_ptr<Language::Loop> token)
    {token != nullptr ? CompileForLoop(*token) : nop;}

    void Compiler::CompileForLoop(Language::Loop& token)
    {
        CompileVariable(token.StartVariable);
        uint16_t pointer = mCodeBuilder.Size() + 1;
        CompileExpression(token.Condition);
        Compile(token.mTokens);
        CompileExpression(token.Iteration);
        mCodeBuilder.JEQ(pointer);
    }

    void Compiler::CompileExpression(std::shared_ptr<Language::Expression> token)
    {token != nullptr ? CompileExpression(*token) : nop;}

    void Compiler::CompileExpression(Language::Expression& token)
    {
        mCodeBuilder.NOP();

        /// TODO: --- implement ---
    }


}