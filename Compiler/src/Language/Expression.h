#ifndef __LANGUAGE__EXPRESSION_H_
#define __LANGUAGE__EXPRESSION_H_ 1

#include "AbstractSyntaxTree.h"
#include "Language.h"
#include "Variable.h"
#include <stdint.h>
#include "Token.h"
#include <string>
#include <memory>

namespace Language
{

    enum class ExpressionType
    {
        Invalid = -1,
        Condition,
        Assignment,
        FunctionCall,
        ReturnStatement,
        KeywordExpression,
        ArithmeticOperation,
    };

    /**
     * @brief Get the string representation of an expression type
     * 
     * @param type The expression type that will be "stringified"
     * 
     * @return The string representation of `type`
     */
    const char* ExpressionTypeToString(ExpressionType type);

    struct Expression : public Token
    {
        ExpressionType Type;
        std::string ReturnValue;
        VariableType ReturnType;
        std::string ReturnVariableName;
        SourceTokenList mInnerTokens;

        AbstractSyntaxTree AST;

        // since cases like `myValue = getValue();` can exist
        // and they can split up into two expressions like in
        // this case a function call and a value assignment
        // every expression will have a "list" of expressions
        // that need to be done before this one
        std::vector<Expression> mInnerExpressions;

        /**
         * @brief Construct a new `Expression` object
         */
        Expression();

        /**
         * @brief Identify the type of this expression
         */
        void Identify(TokenList& localTokens, TokenList& globalTokens = Language::EmptyTokenList);

    private:
        void ParseTokens(SourceTokenList& tokens, TokenList& localTokens, TokenList& globalTokens);

    private:
        /**
         * @brief Check if the provided list contains any tokens that split up expressions
         * 
         * @param tokens The list of tokens
         * 
         * @returns `true` if there are splitting tokens, `false` if there aren't
         */
        bool CheckExpressionDividers(SourceTokenList& tokens);

    private:
        void ParseCondition(SourceTokenList& tokens, TokenList& localTokens, TokenList& globalTokens);
        void ParseAssignment(SourceTokenList& tokens, TokenList& localTokens, TokenList& globalTokens);
        void ParseFunctionCall(SourceTokenList& tokens, TokenList& localTokens, TokenList& globalTokens);
        void ParseReturnStatement(SourceTokenList& tokens, TokenList& localTokens, TokenList& globalTokens);
        void ParseKeywordExpression(SourceTokenList& tokens, TokenList& localTokens, TokenList& globalTokens);
        void ParseArithmeticOperation(SourceTokenList& tokens, TokenList& localTokens, TokenList& globalTokens);
    };

}

#endif