#include "CompiledScript.h"
#include "../StringUtil.h"
#include "../Logging.h"
#include "Compiler.h"
#include <memory.h>

#define nop (void)0

namespace Binaries
{

    Compiler::Compiler(CompiledScript& script)
        : mCode(nullptr), mCodeLength(0), mCodeBuilder(), mScript(script), mVarTable()
    {
    }

    void Compiler::ClearCompiledCode()
    {
        if (mCode != nullptr)
            free(mCode);

        mCode = nullptr;
        mCodeLength = 0;
        mVarTable.Clear();
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


    uint64_t Compiler::PredictCodeSize(TokenList& tokens)
    {
        Compiler c = Compiler(mScript);
        c.Compile(tokens);
        c.FinalizeCompilation();
        size_t size = c.GetCompiledCodeSize();
        c.ClearCompiledCode();
        return size;
    }

    uint64_t Compiler::PredictCodeSize(std::shared_ptr<Language::Expression> token)
    {
        if (token != nullptr)
            return PredictCodeSize(*token);
        return 0;
    }

    uint64_t Compiler::PredictCodeSize(Language::Expression& token)
    {
        Compiler c = Compiler(mScript);
        c.CompileExpression(token);
        c.FinalizeCompilation();
        size_t size = c.GetCompiledCodeSize();
        c.ClearCompiledCode();
        return size;
    }

    uint64_t Compiler::GetIP()
    {
        return mCodeBuilder.Size() + 1;
    }

    void Compiler::CompileVariable(std::shared_ptr<Language::Variable> token)
    {token != nullptr ? CompileVariable(*token) : nop;}

    uint8_t* GetVariableValueAsData(std::string value, Language::VariableType type);

    void Compiler::CompileVariable(Language::Variable& token)
    {
        size_t size = Language::VariableTypeBytes(token.Type);
        if (token.Type == Language::VariableType::String)
            size = token.Value.length() + 1; // plus the nullchar at the end

        mCodeBuilder.MEM(size, token.ID);
        uint8_t* data = GetVariableValueAsData(token.Value, token.Type);
        mCodeBuilder.SET(token.ID, size, data);
        free(data);

        mVarTable.Add(token);
    }

    void Compiler::CompileIfElseStatement(std::shared_ptr<Language::IfElseStatement> token)
    {token != nullptr ? CompileIfElseStatement(*token) : nop;}

    void Compiler::CompileIfElseStatement(Language::IfElseStatement& token)
    {
        int codeSize = GetIP() + PredictCodeSize(token.mTokens);
        int ptr = GetIP();
        mCodeBuilder.JMPOFF(codeSize);
        Compile(token.mTokens);
        int nextPtr = PredictCodeSize(token.Condition) + 1;
        mCodeBuilder.CCF();
        mCodeBuilder.JMPOFF(nextPtr);
        CompileExpression(token.Condition);
        mCodeBuilder.JEQ(ptr);
        mCodeBuilder.SCF();
        mCodeBuilder.CCV();

        for (auto& otherStatement : token.ChainedStatements)
            CompileIfElseStatement(otherStatement);
    }

    void Compiler::CompileDoWhileLoop(std::shared_ptr<Language::Loop> token)
    {token != nullptr ? CompileDoWhileLoop(*token) : nop;}

    void Compiler::CompileDoWhileLoop(Language::Loop& token)
    {
        uint16_t pointer = GetIP();
        Compile(token.mTokens);
        CompileExpression(token.Condition);
        mCodeBuilder.JEQ(pointer);
        mCodeBuilder.CCV();
    }

    void Compiler::CompileWhileLoop(std::shared_ptr<Language::Loop> token)
    {token != nullptr ? CompileWhileLoop(*token) : nop;}

    void Compiler::CompileWhileLoop(Language::Loop& token)
    {
        int offset = PredictCodeSize(token.mTokens) + 1;
        mCodeBuilder.JMPOFF(offset);
        uint16_t pointer = GetIP();
        Compile(token.mTokens);
        CompileExpression(token.Condition);
        mCodeBuilder.JEQ(pointer);
        mCodeBuilder.CCV();
    }

    void Compiler::CompileForLoop(std::shared_ptr<Language::Loop> token)
    {token != nullptr ? CompileForLoop(*token) : nop;}

    void Compiler::CompileForLoop(Language::Loop& token)
    {
        CompileVariable(token.StartVariable);
        int offset = PredictCodeSize(token.mTokens) + 1;
        mCodeBuilder.JMPOFF(offset);
        uint16_t pointer = GetIP();
        Compile(token.mTokens);
        CompileExpression(token.Condition);
        mCodeBuilder.PUSHCV();
        CompileExpression(token.Iteration);
        mCodeBuilder.POPCV();
        mCodeBuilder.JEQ(pointer);
        mCodeBuilder.CCV();

        if (token.StartVariable)
            mVarTable.Remove(*token.StartVariable);
    }

    void Compiler::CompileExpression(std::shared_ptr<Language::Expression> token)
    {token != nullptr ? CompileExpression(*token) : nop;}

    index_t Compiler::Has(Language::TokenType type, std::string name)
    {
        Section* section;

        if (type == Language::TokenType::Variable)
            section = (Section*)mScript.GetSection(name, SectionType::Variable);
        else if (type == Language::TokenType::Field)
            section = (Section*)mScript.GetSection(name, SectionType::Field);
        else if (type == Language::TokenType::Function)
            section = (Section*)mScript.GetSection(name, SectionType::Function);
        else
            return -1;

        if (section == nullptr)
            return -1;

        return section->ID;
    }

    index_t Compiler::Has(Language::TokenType type, uint64_t id)
    {
        Section* section;

        if (type == Language::TokenType::Variable)
            section = (Section*)mScript.GetSection(id, SectionType::Variable);
        else if (type == Language::TokenType::Field)
            section = (Section*)mScript.GetSection(id, SectionType::Field);
        else if (type == Language::TokenType::Function)
            section = (Section*)mScript.GetSection(id, SectionType::Function);
        else
            return -1;

        if (section == nullptr)
            return -1;

        return section->ID;
    }

    void Compiler::CompileExpression(Language::Expression& token)
    {
        auto astRoot = token.mAST.GetRoot();

        if (token.Type == Language::ExpressionType::Invalid || token.Type == Language::ExpressionType::None);
        {
            mCodeBuilder.NOP();
            mCodeBuilder.KILL(-69420);
            return;
        }

        if (token.Type == Language::ExpressionType::Condition)
        {
            /// TODO: --- implement ---

            CompileAstNode(astRoot, true);
            return;
        }

        if (token.Type == Language::ExpressionType::Assignment)
        {
            CompileAssignment(astRoot);
            return;
        }

        if (token.Type == Language::ExpressionType::FunctionCall)
        {
            /// TODO: --- implement ---

            CompileAstNode(astRoot, true);
            return;
        }

        if (token.Type == Language::ExpressionType::ReturnStatement)
        {
            /// TODO: --- implement ---

            CompileAstNode(astRoot, true);
            return;
        }

        if (token.Type == Language::ExpressionType::KeywordExpression)
        {
            /// TODO: --- implement ---

            CompileAstNode(astRoot, true);
            return;
        }

        if (token.Type == Language::ExpressionType::ArithmeticOperation)
        {
            /// TODO: --- implement ---

            CompileAstNode(astRoot, true);
            return;
        }
    }

    void Compiler::CompileAstNode(TreeNodeObject node, bool isRoot = false)
    {
        /// TODO: --- implement ---

        mCodeBuilder.NOP();
    }

    void Compiler::CompileAssignment(TreeNodeObject root)
    {
        auto leftToken = root->left->contents;
        if (leftToken.mType != SourceToken::Type::Text)
            Logging::LogErrorExit(stringf("Line %d: Invalid assignment, no variable name given, instead found '%s'", leftToken.LineNumber, leftToken.Contents.c_str()));

        NameLookupTable* lookupTable = (NameLookupTable*)mScript.GetNameLookupTable();
        std::string varName = leftToken.Contents;
        auto varID = lookupTable->Lookup(varName);
        if (varID == -1) // maybe a local variable?
            mVarTable.Lookup(varName);

        if (varID == -1) // ok nah, wtf is the script writer doing
            Logging::LogErrorExit(stringf("Line %d: No variable with name '%s' found", leftToken.LineNumber, leftToken.Contents.c_str()));

        //
    }

    uint8_t* GetVariableValueAsData(std::string value, Language::VariableType type)
    {
        uint8_t* data = nullptr;

        if (type == Language::VariableType::Bool)
        {
            data = (uint8_t*)malloc(1);
            *data = value == "true";
        }

        if (type == Language::VariableType::Float)
        {
            double dataValue = 0;

            if (!value.empty())
                dataValue = std::stod(value);

            data = (uint8_t*)malloc(sizeof(double));
            memcpy(data, &dataValue, sizeof(double));
        }

        if (type == Language::VariableType::GameObject)
        {
            int64_t dataValue = 0;
            if (value == Language::Keywords[KeywordIndex_attached].Name)
                dataValue = -1;
            else if (value == Language::Keywords[KeywordIndex_null].Name)
                dataValue = 0;
            else
            {
                ///TODO: --- implement getting id of gameobjects ---
            }

            data = (uint8_t*)malloc(sizeof(int64_t));
            memcpy(data, &dataValue, sizeof(int64_t));
        }

        if (type == Language::VariableType::Int || type == Language::VariableType::Uint)
        {
            int64_t dataValue = 0;

            if (!value.empty())
                dataValue = std::stoll(value);

            data = (uint8_t*)malloc(sizeof(int64_t));
            memcpy(data, &dataValue, sizeof(int64_t));
        }

        if (type == Language::VariableType::String)
        {
            data = (uint8_t*)malloc(value.length() + 1);
            memset(data, 0, value.length() + 1);
            memcpy(data, value.data(), value.length());
        }

        return data;
    }

}