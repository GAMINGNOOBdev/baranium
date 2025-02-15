#include "../MemoryManager.h"
#include "CodeBuilder.h"
#include "CompiledScript.h"
#include "../StringUtil.h"
#include "../Logging.h"
#include "Compiler.h"
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <memory.h>

#define nop (void)0

namespace Binaries
{

    Compiler::Compiler(CompiledScript& script)
        : mCode(nullptr), mCodeLength(0), mVarTable(), mScript(script), mCodeBuilder()
    {
    }

    void Compiler::ClearCompiledCode()
    {
        if (mCode != nullptr)
            MemoryManager::deallocate(mCode);

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

            case Language::VariableType::Object:
            {
                int64_t objID;
                if (__src.empty())
                    objID = 0;
                else if (__src == Language::Keywords[KeywordIndex_null].Name)
                    objID = 0;
                else if (__src == Language::Keywords[KeywordIndex_attached].Name)
                    objID = -1;
                else
                    objID = StrGetNumber(__src);
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
                    iValue = (int32_t)StrGetNumber(__src);
                memcpy(dest, &iValue, sizeof(int32_t));
                break;
            }

            case Language::VariableType::Uint:
            {
                uint32_t uValue = 0;
                if (!__src.empty())
                    uValue = (uint32_t)StrGetNumber(__src);
                memcpy(dest, &uValue, sizeof(uint32_t));
                break;
            }

        }
    }

    void Compiler::CompileVariables(VariableList& variables)
    {
        for (size_t var = variables.size(); var > 0; var--)
        {
            auto& entry = variables[var-1];
            CompileVariable(entry);
            mCodeBuilder.POPVAR(entry->ID);
        }
    }

    void Compiler::ClearVariables(VariableList& variables)
    {
        if (mCodeBuilder.ReturnedFromExecution())
            return;

        for (auto& var : variables)
        {
            if (!var)
                continue;

            mCodeBuilder.FEM(var->ID);
            mVarTable.Remove(*var);
        }
    }

    void Compiler::Compile(TokenList& tokens)
    {
        for (size_t i = 0; i < tokens.size(); i++)
        {
            auto& token = tokens.at(i);

            switch(token->mTokenType)
            {
            default:
                continue;

            case Language::TokenType::Function:
                Logging::LogErrorExit("Trying to compile function inside function, bruh");

            case Language::TokenType::Field:
                Logging::LogErrorExit("Fields should not be inside functions my man");

            case Language::TokenType::Variable:
                CompileVariable(std::static_pointer_cast<Language::Variable>(token));
                continue;

            case Language::TokenType::Expression:
                CompileExpression(std::static_pointer_cast<Language::Expression>(token));
                continue;

            case Language::TokenType::IfElseStatement:
                CompileIfElseStatement(std::static_pointer_cast<Language::IfElseStatement>(token));
                continue;

            case Language::TokenType::DoWhileLoop:
                CompileDoWhileLoop(std::static_pointer_cast<Language::Loop>(token));
                continue;

            case Language::TokenType::WhileLoop:
                CompileWhileLoop(std::static_pointer_cast<Language::Loop>(token));
                continue;

            case Language::TokenType::ForLoop:
                CompileForLoop(std::static_pointer_cast<Language::Loop>(token));
                continue;
            }
        }
    }

    void Compiler::FinalizeCompilation()
    {
        if (mCode != nullptr)
            MemoryManager::deallocate(mCode);

        mCodeLength = mCodeBuilder.Size();
        mCode = (uint8_t*)MemoryManager::allocate(mCodeLength+1);
        memset(mCode, 0, mCodeLength+1);
        memcpy(mCode, mCodeBuilder.Data(), mCodeLength);

        mVarTable.Clear();
    }

    uint64_t Compiler::PredictCodeSize(TokenList& tokens)
    {
        Compiler c = Compiler(mScript);
        c.mVarTable = mVarTable;
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
        c.mVarTable = mVarTable;
        c.CompileExpression(token);
        c.FinalizeCompilation();
        size_t size = c.GetCompiledCodeSize();
        c.ClearCompiledCode();
        return size;
    }

    uint64_t Compiler::GetIP()
    {
        return mCodeBuilder.Size();
    }

    void Compiler::CompileVariable(std::shared_ptr<Language::Variable> token)
    {token != nullptr ? CompileVariable(*token) : nop;}

    uint8_t* GetVariableValueAsData(std::string value, Language::VariableType type);

    void Compiler::CompileVariable(Language::Variable& token)
    {
        size_t size = Language::VariableTypeBytes(token.Type);
        if (token.Type == Language::VariableType::String)
            size = token.Value.length() + 1; // plus the nullchar at the end

        mCodeBuilder.MEM(size, (uint8_t)token.Type, token.ID);
        if (token.Type != Language::VariableType::Object)
        {
            uint8_t* data = GetVariableValueAsData(token.Value, token.Type);
            mCodeBuilder.SET(token.ID, size, data);
            MemoryManager::deallocate(data);
        }
        else
        {
            if (token.Value == Language::Keywords[KeywordIndex_attached].Name)
                mCodeBuilder.PUSH(-1);
            else if (token.Value == Language::Keywords[KeywordIndex_null].Name)
                mCodeBuilder.PUSH(0);
            else if (StrIsNumber(token.Value))
                mCodeBuilder.PUSH(StrGetNumber(token.Value));
            else
            {
                auto varID = GetID(token.Value);
                mCodeBuilder.PUSHVAR(varID);
                mCodeBuilder.POPVAR(token.ID);
            }
            mCodeBuilder.POPVAR(token.ID);
        }

        mVarTable.Add(token);
    }

    void Compiler::CompileIfElseStatement(std::shared_ptr<Language::IfElseStatement> token)
    {token != nullptr ? CompileIfElseStatement(*token) : nop;}

    void Compiler::CompileIfElseStatement(Language::IfElseStatement& token)
    {
        int codeSize = PredictCodeSize(token.mTokens);
        mCodeBuilder.SCF();
        CompileExpression(token.Condition);
        mCodeBuilder.PUSHCV();
        mCodeBuilder.ICV();
        mCodeBuilder.JMPCOFF(codeSize);
        Compile(token.mTokens);

        for (auto& otherStatement : token.ChainedStatements)
            CompileIfElseSubStatement(otherStatement);

        mCodeBuilder.POPCV();
        mCodeBuilder.CCF();
    }

    void Compiler::CompileIfElseSubStatement(Language::IfElseStatement& token)
    {
        if (token.Condition == nullptr)
        {
            CompileElseStatement(token);
            return;
        }

        int codeSize = PredictCodeSize(token.mTokens);
        int conditionSize = PredictCodeSize(token.Condition) + 3; // + 3 because POPCV, PUSHCV and ICV are one byte instructions each
        mCodeBuilder.SCF();
        mCodeBuilder.POPCV();
        mCodeBuilder.PUSHCV();
        mCodeBuilder.JMPCOFF(conditionSize);
        mCodeBuilder.POPCV();
        CompileExpression(token.Condition);
        mCodeBuilder.PUSHCV();
        mCodeBuilder.ICV();
        mCodeBuilder.JMPCOFF(codeSize);
        Compile(token.mTokens);
    }

    void Compiler::CompileElseStatement(Language::IfElseStatement& token)
    {
        if (token.Condition != nullptr)
            return;

        int codeSize = PredictCodeSize(token.mTokens);
        mCodeBuilder.SCF();
        mCodeBuilder.POPCV();
        mCodeBuilder.PUSHCV();
        mCodeBuilder.JMPCOFF(codeSize);
        Compile(token.mTokens);
    }

    void Compiler::CompileDoWhileLoop(std::shared_ptr<Language::Loop> token)
    {token != nullptr ? CompileDoWhileLoop(*token) : nop;}

    void Compiler::CompileDoWhileLoop(Language::Loop& token)
    {
        uint64_t pointer = GetIP();
        Compile(token.mTokens);
        CompileExpression(token.Condition);
        mCodeBuilder.SCF();
        mCodeBuilder.CCV();
        mCodeBuilder.JMPC(pointer);
        mCodeBuilder.CCV();
        mCodeBuilder.CCF();
    }

    void Compiler::CompileWhileLoop(std::shared_ptr<Language::Loop> token)
    {token != nullptr ? CompileWhileLoop(*token) : nop;}

    void Compiler::CompileWhileLoop(Language::Loop& token)
    {
        int offset = PredictCodeSize(token.mTokens);
        mCodeBuilder.JMPOFF(offset);
        uint64_t pointer = GetIP();
        Compile(token.mTokens);
        mCodeBuilder.SCF();
        mCodeBuilder.CCV();
        CompileExpression(token.Condition);
        mCodeBuilder.JMPC(pointer);
        mCodeBuilder.CCV();
        mCodeBuilder.CCF();
    }

    void Compiler::CompileForLoop(std::shared_ptr<Language::Loop> token)
    {token != nullptr ? CompileForLoop(*token) : nop;}

    void Compiler::CompileForLoop(Language::Loop& token)
    {
        CompileVariable(token.StartVariable); // either a variable was declared
        CompileExpression(token.StartExpr);   // or a starting expression
        int offset = PredictCodeSize(token.mTokens) + PredictCodeSize(token.Iteration);
        mCodeBuilder.JMPOFF(offset);
        uint64_t pointer = GetIP();
        Compile(token.mTokens);
        CompileExpression(token.Iteration);
        mCodeBuilder.SCF();
        mCodeBuilder.CCV();
        CompileExpression(token.Condition);
        mCodeBuilder.JMPC(pointer);

        if (token.StartVariable)
        {
            mVarTable.Remove(*token.StartVariable);
            mCodeBuilder.FEM(token.StartVariable->ID);
        }
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

    index_t Compiler::GetID(std::string name, int lineNumber)
    {
        index_t varID = -1;
        NameLookupTable* lookupTable = (NameLookupTable*)mScript.GetNameLookupTable();
        if (lookupTable)
            varID = lookupTable->Lookup(name);

        if (varID == -1) // maybe a local variable?
            varID = mVarTable.Lookup(name);

        if (varID == -1) // ok, the script writer really doesn't know
            Logging::LogErrorExit(stringf("Line %d: No variable with name '%s' found", lineNumber, name.c_str()));

        return varID;
    }

    void Compiler::CompileExpression(Language::Expression& token)
    {
        auto astRoot = token.mAST.GetRoot();

        if (token.Type == Language::ExpressionType::Invalid || token.Type == Language::ExpressionType::None || astRoot == nullptr)
        {
            mCodeBuilder.NOP();
            mCodeBuilder.KILL(-69420);
            return;
        }

        if (token.Type == Language::ExpressionType::Condition)
        {
            CompileCondition(astRoot);
            return;
        }

        if (token.Type == Language::ExpressionType::Assignment)
        {
            CompileAssignment(astRoot);
            return;
        }

        if (token.Type == Language::ExpressionType::FunctionCall)
        {
            CompileFunctionCall(token);
            return;
        }

        if (token.Type == Language::ExpressionType::ReturnStatement)
        {
            CompileReturnStatement(token);
            return;
        }

        if (token.Type == Language::ExpressionType::KeywordExpression)
        {
            CompileKeywordExpression(token);
            return;
        }

        if (token.Type == Language::ExpressionType::ArithmeticOperation)
        {
            CompileArithmeticOperation(astRoot, true);
            return;
        }
    }

    void Compiler::CompileAstNode(TreeNodeObject node, bool isRoot)
    {
        if (node == nullptr) return;

        if (node->subNodes.size() > 0)
        {
            CompileFunctionCall(node);
            return;
        }

        auto token = node->contents;

        if (token.mType == SourceToken::Type::Number && !isRoot)
        {
            uint64_t value = 0;
            if (token.Contents.find_first_of('.') != std::string::npos)
            {
                float val = std::stof(token.Contents);
                mCodeBuilder.PushFloatValue(val);
            }
            else
            {
                value = StrGetNumber(token.Contents);
                mCodeBuilder.PushIntValue(value);
            }
        }
        else if (token.mType == SourceToken::Type::Keyword && !isRoot)
        {
            if (token.KeywordIndex == KeywordIndex_false)
                mCodeBuilder.PushBoolValue(false);
            else if (token.KeywordIndex == KeywordIndex_true)
                mCodeBuilder.PushBoolValue(true);
            else
                Logging::LogErrorExit(stringf("Line %d: Invalid keyword '%s'", token.LineNumber, token.Contents.c_str()));
        }
        else if (token.mType == SourceToken::Type::Null && !isRoot)
            mCodeBuilder.PushUintValue(0);
        else if (token.mType == SourceToken::Type::Text && !isRoot)
            mCodeBuilder.PUSHVAR(GetID(token.Contents, token.LineNumber));
        else if (token.mType == SourceToken::Type::DoubleQuote && !isRoot)
            mCodeBuilder.PushStringValue(node->left->contents.Contents);

        if (token.mType == SourceToken::Type::Plus          || token.mType == SourceToken::Type::Minus      ||
            token.mType == SourceToken::Type::Asterisk      || token.mType == SourceToken::Type::Slash      ||
            token.mType == SourceToken::Type::Modulo        || token.mType == SourceToken::Type::And        ||
            token.mType == SourceToken::Type::Or            || token.mType == SourceToken::Type::Tilde      ||
            token.mType == SourceToken::Type::MinusMinus    || token.mType == SourceToken::Type::PlusPlus   ||
            token.mType == SourceToken::Type::Caret)
            CompileArithmeticOperation(node);

        if (token.mType == SourceToken::Type::EqualSign    || token.mType == SourceToken::Type::ModEqual  ||
            token.mType == SourceToken::Type::DivEqual     || token.mType == SourceToken::Type::MulEqual  ||
            token.mType == SourceToken::Type::MinusEqual   || token.mType == SourceToken::Type::PlusEqual ||
            token.mType == SourceToken::Type::AndEqual     || token.mType == SourceToken::Type::OrEqual   ||
            token.mType == SourceToken::Type::XorEqual)
            CompileAssignment(node);

        if (token.mType == SourceToken::Type::EqualTo   || token.mType == SourceToken::Type::NotEqual       ||
            token.mType == SourceToken::Type::LessEqual || token.mType == SourceToken::Type::GreaterEqual   ||
            token.mType == SourceToken::Type::LessThan  || token.mType == SourceToken::Type::GreaterThan    ||
            token.mType == SourceToken::Type::AndAnd    || token.mType == SourceToken::Type::OrOr)
        {
            CompileCondition(node);
            mCodeBuilder.PUSHCV();
        }
    }

    void Compiler::CompileAssignment(TreeNodeObject root)
    {
        if (root == nullptr) return;

        auto leftToken = root->left->contents;
        if (leftToken.mType != SourceToken::Type::Text)
            Logging::LogErrorExit(stringf("Line %d: Invalid assignment, no variable name given, instead found '%s'", leftToken.LineNumber, leftToken.Contents.c_str()));

        std::string varName = leftToken.Contents;
        auto varID = GetID(varName, leftToken.LineNumber);

        CompileAstNode(root->right);

        if (root->contents.mType != SourceToken::Type::EqualSign)
            mCodeBuilder.PUSHVAR(varID);

        if (root->contents.mType == SourceToken::Type::ModEqual)
            mCodeBuilder.MOD();
        if (root->contents.mType == SourceToken::Type::DivEqual)
            mCodeBuilder.DIV();
        if (root->contents.mType == SourceToken::Type::MulEqual)
            mCodeBuilder.MUL();
        if (root->contents.mType == SourceToken::Type::MinusEqual)
            mCodeBuilder.SUB();
        if (root->contents.mType == SourceToken::Type::PlusEqual)
            mCodeBuilder.ADD();
        if (root->contents.mType == SourceToken::Type::AndEqual)
            mCodeBuilder.AND();
        if (root->contents.mType == SourceToken::Type::OrEqual)
            mCodeBuilder.OR();
        if (root->contents.mType == SourceToken::Type::XorEqual)
            mCodeBuilder.XOR();

        mCodeBuilder.POPVAR(varID);
    }

    void Compiler::CompileReturnStatement(Language::Expression& expression)
    {
        if (expression.ReturnExpression != nullptr)
        {
            CompileAstNode(expression.ReturnExpression->mAST.GetRoot(), true);
        }
        else if (!expression.ReturnVariableName.empty())
        {
            std::string varName = expression.ReturnVariableName;
            auto varID = GetID(varName, expression.LineNumber);
            mCodeBuilder.PUSHVAR(varID);
        }
        else if (expression.ReturnType != Language::VariableType::Void)
        {
            Language::Variable var = Language::Variable();
            var.Type = expression.ReturnType;
            var.Value = expression.ReturnValue;
            CompileVariable(var);
            mCodeBuilder.PUSHVAR(var.ID);
            mCodeBuilder.FEM(var.ID);
            mVarTable.Remove(var);
        }

        // clear all currently allocated variables
        for (auto& var : mVarTable.GetAllEntries())
            mCodeBuilder.FEM(var.ID);

        mCodeBuilder.RET();
    }

    void Compiler::CompileArithmeticOperation(TreeNodeObject root, bool isRoot)
    {
        if (root == nullptr) return;

        auto lhs = root->left;
        auto rhs = root->right;

        if (root->contents.mType == SourceToken::Type::MinusMinus && lhs->contents.mType == SourceToken::Type::Text)
        {
            // unfortunately i don't know any better way right now
            // to do this other than having duplicated code, sorry
            std::string varName = lhs->contents.Contents;
            auto varID = GetID(varName, lhs->contents.LineNumber);
            mCodeBuilder.PUSHVAR(varID);
            mCodeBuilder.PushIntValue(1);
            mCodeBuilder.SUB();
            mCodeBuilder.POPVAR(varID);
            return;
        }
        if (root->contents.mType == SourceToken::Type::PlusPlus && lhs->contents.mType == SourceToken::Type::Text)
        {
            // unfortunately i don't know any better way right now
            // to do this other than having duplicated code, sorry
            std::string varName = lhs->contents.Contents;
            auto varID = GetID(varName, lhs->contents.LineNumber);
            mCodeBuilder.PUSHVAR(varID);
            mCodeBuilder.PushIntValue(1);
            mCodeBuilder.ADD();
            mCodeBuilder.POPVAR(varID);
            return;
        }

        if (isRoot)
        {
            // having an arithmetic operation as a normal expression can
            // be harmful to the stack so we have to be careful and avoid
            // compilation of arithmetic expressions if they are the root
            // expression, even if a subexpression is something else
            mCodeBuilder.NOP();
            return;
        }

        if (lhs)
            CompileAstNode(lhs);
        else
            mCodeBuilder.PushIntValue(0);

        if (rhs)
            CompileAstNode(rhs);
        else
            mCodeBuilder.PushIntValue(0);

        if (root->contents.mType == SourceToken::Type::Modulo)
            mCodeBuilder.MOD();
        if (root->contents.mType == SourceToken::Type::Slash)
            mCodeBuilder.DIV();
        if (root->contents.mType == SourceToken::Type::Asterisk)
            mCodeBuilder.MUL();
        if (root->contents.mType == SourceToken::Type::Minus)
            mCodeBuilder.SUB();
        if (root->contents.mType == SourceToken::Type::Plus)
            mCodeBuilder.ADD();
        if (root->contents.mType == SourceToken::Type::And)
            mCodeBuilder.AND();
        if (root->contents.mType == SourceToken::Type::Or)
            mCodeBuilder.OR();
        if (root->contents.mType == SourceToken::Type::Caret)
            mCodeBuilder.XOR();
        if (root->contents.mType == SourceToken::Type::BitshiftLeft)
            mCodeBuilder.SHFTL();
        if (root->contents.mType == SourceToken::Type::BitshiftRight)
            mCodeBuilder.SHFTR();
    }

    void Compiler::CompileCondition(TreeNodeObject root)
    {
        if (root == nullptr) return;

        auto lhs = root->left;
        auto rhs = root->right;

        if (lhs)
            CompileAstNode(lhs);
        else
            mCodeBuilder.PushIntValue(0);

        if (rhs)
            CompileAstNode(rhs);
        else
            mCodeBuilder.PushIntValue(0);

        if (root->contents.mType == SourceToken::Type::AndAnd)
            mCodeBuilder.CMPC(CMP_AND);
        else if (root->contents.mType == SourceToken::Type::OrOr)
            mCodeBuilder.CMPC(CMP_OR);
        else
            mCodeBuilder.CMP(GetCompareMethod(root->contents.mType));
    }

    void Compiler::CompileKeywordExpression(Language::Expression& expression)
    {
        std::string keyword = std::string(expression.mAST.GetRoot()->contents.Contents);

        if (expression.ReturnValue == Language::Keywords[KeywordIndex_null].Name)
        {
            // it wouldn't make sense to do any sort of operation on a non-existent object
            mCodeBuilder.NOP();
            return;
        }
        else if (expression.ReturnValue == Language::Keywords[KeywordIndex_attached].Name)
            mCodeBuilder.PushIntValue(-1);
        else
        {
            auto id = GetID(expression.ReturnValue, expression.LineNumber);
            ///TODO: check if variable is an signed/unsigned integer or an object, since any other type doesn't make sense
            mCodeBuilder.PUSHVAR(id);
        }

        if (keyword == Language::Keywords[KeywordIndex_instantiate].Name)
        {
            mCodeBuilder.INSTANTIATE();
            return;
        }
        else if (keyword == Language::Keywords[KeywordIndex_delete].Name)
        {
            mCodeBuilder.DELETE();
            return;
        }
        else if (keyword == Language::Keywords[KeywordIndex_attach].Name)
        {
            mCodeBuilder.ATTACH();
            return;
        }
        else if (keyword == Language::Keywords[KeywordIndex_detach].Name)
        {
            mCodeBuilder.DETACH();
            return;
        }

        Logging::LogErrorExit(stringf("Line %d: Unknown keyword or maybe variable doesn't exist?", expression.LineNumber));
    }

    void Compiler::CompileFunctionCall(Language::Expression& expression)
    {
        CompileFunctionCall(expression.mAST.GetRoot());
    }

    void Compiler::CompileFunctionCall(TreeNodeObject node)
    {
        std::string functionName = std::string(node->contents.Contents);
        auto id = GetID(functionName, node->contents.LineNumber);
        for (auto& subNode : node->subNodes)
            CompileAstNode(subNode, false);

        mCodeBuilder.CALL(id);
    }

    uint8_t Compiler::GetCompareMethod(SourceToken::Type type)
    {
        switch (type)
        {
            default:
            case SourceToken::Type::EqualTo:
                return CMP_EQUAL;
            case SourceToken::Type::NotEqual:
                return CMP_NOTEQUAL;
            case SourceToken::Type::LessThan:
                return CMP_LESS_THAN;
            case SourceToken::Type::LessEqual:
                return CMP_LESS_EQUAL;
            case SourceToken::Type::GreaterThan:
                return CMP_GREATER_THAN;
            case SourceToken::Type::GreaterEqual:
                return CMP_GREATER_EQUAL;
        }
        return CMP_EQUAL;
    }

    uint8_t* GetVariableValueAsData(std::string value, Language::VariableType type)
    {
        uint8_t* data = nullptr;

        if (type == Language::VariableType::Bool)
        {
            data = (uint8_t*)MemoryManager::allocate(1);
            if (!data) return nullptr;
            *data = value == "true";
        }

        if (type == Language::VariableType::Float)
        {
            double dataValue = 0;

            if (!value.empty())
                dataValue = std::stod(value);

            data = (uint8_t*)MemoryManager::allocate(sizeof(double));
            memcpy(data, &dataValue, sizeof(double));
        }

        if (type == Language::VariableType::Object)
            return nullptr;

        if (type == Language::VariableType::Int || type == Language::VariableType::Uint)
        {
            int64_t dataValue = 0;

            if (!value.empty())
                dataValue = StrGetNumber(value);

            data = (uint8_t*)MemoryManager::allocate(sizeof(int64_t));
            memcpy(data, &dataValue, sizeof(int64_t));
        }

        if (type == Language::VariableType::String)
        {
            data = (uint8_t*)MemoryManager::allocate(value.length() + 1);
            memset(data, 0, value.length() + 1);
            memcpy(data, value.data(), value.length());
        }

        return data;
    }

}
