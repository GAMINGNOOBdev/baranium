#ifndef __BINARIES__COMPILER_H_
#define __BINARIES__COMPILER_H_ 1

#include "../Language/Types.h"
#include "VariableTable.h"
#include "CodeBuilder.h"
#include <stdint.h>
#include <vector>

namespace Binaries
{

    struct CompiledScript;

    /**
     * @brief A class that compiles tokens into executable binary code
     */
    struct Compiler
    {
        /**
         * @brief Construct a new `Compiler`
         * 
         * @param script The script that will be compiled
         */
        Compiler(CompiledScript& script);

        /**
         * @brief Clear the currently compiled code
         * 
         * @note This should be called after compiling a function and moving onto the next
         */
        void ClearCompiledCode();

        /**
         * @brief Get the compiled code
         * 
         * @returns A pointer to the compiled code data
         */
        uint8_t* GetCompiledCode();

        /**
         * @brief Get the size in bytes of the compiled code
         * 
         * @returns The size of the compiled code
         */
        size_t GetCompiledCodeSize();

        /**
         * @brief Small utility function of a compiler that copies the value of a variable into a destination buffer
         * 
         * @param __dst The destination buffer that will hold the data
         * @param __src The string representation of the data
         * @param varType The variable type
         */
        void CopyVariableData(void* __dst, std::string __src, Language::VariableType varType);

        /**
         * @brief Compile a given list of variables
         * 
         * @param variables Variable list
         */
        void CompileVariables(VariableList& variables);

        /**
         * @brief Clear a given list of variables
         * 
         * @param variables Variable list
         */
        void ClearVariables(VariableList& variables);

        /**
         * @brief Compile the given tokens into binary
         * 
         * @param tokens The tokens that will be compiled
         */
        void Compile(TokenList& tokens);

        /**
         * @brief Finalize the compilation of tokens
         */
        void FinalizeCompilation();

    private:
        void CompileVariable(std::shared_ptr<Language::Variable> token);
        void CompileVariable(Language::Variable& token);
        void CompileExpression(std::shared_ptr<Language::Expression> token);
        void CompileExpression(Language::Expression& token);
        void CompileIfElseStatement(std::shared_ptr<Language::IfElseStatement> token);
        void CompileIfElseStatement(Language::IfElseStatement& token);
        void CompileDoWhileLoop(std::shared_ptr<Language::Loop> token);
        void CompileDoWhileLoop(Language::Loop& token);
        void CompileWhileLoop(std::shared_ptr<Language::Loop> token);
        void CompileWhileLoop(Language::Loop& token);
        void CompileForLoop(std::shared_ptr<Language::Loop> token);
        void CompileForLoop(Language::Loop& token);

        void CompileAstNode(TreeNodeObject node, bool isRoot = false);

        index_t Has(Language::TokenType type, std::string name);
        index_t Has(Language::TokenType type, uint64_t id);

        uint64_t PredictCodeSize(TokenList& tokens);
        uint64_t PredictCodeSize(Language::Expression& token);
        uint64_t PredictCodeSize(std::shared_ptr<Language::Expression> token);

        uint64_t GetIP();

    private:
        index_t GetVarID(std::string name, int lineNumber = -1);

    private:
        void CompileAssignment(TreeNodeObject root);
        void CompileReturnStatement(Language::Expression& expression);
        void CompileArithmeticOperation(TreeNodeObject root, bool isRoot = false);
        void CompileCondition(TreeNodeObject root);
        void CompileKeywordExpression(Language::Expression& expression);

    private:
        uint8_t* mCode;
        size_t mCodeLength;
        VariableTable mVarTable;
        CompiledScript& mScript;
        CodeBuilder mCodeBuilder;
    };

}

#endif