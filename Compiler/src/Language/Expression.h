#ifndef __LANGUAGE__EXPRESSION_H_
#define __LANGUAGE__EXPRESSION_H_ 1

#include "Language.h"
#include "Variable.h"
#include <stdint.h>
#include "Token.h"
#include <string>
#include <memory>

using SourceTokenList = std::vector<SourceToken>;
using TokenList = std::vector<std::shared_ptr<Language::Token>>;

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

        /**
         * @returns The string representation of this `Expression`
         */
        std::string ToString() override;

    private:
        void ParseTokens(TokenList& localTokens, TokenList& globalTokens);

    private:
        void ParseCondition(TokenList& localTokens, TokenList& globalTokens);
        void ParseAssignment(TokenList& localTokens, TokenList& globalTokens);
        void ParseFunctionCall(TokenList& localTokens, TokenList& globalTokens);
        void ParseReturnStatement(TokenList& localTokens, TokenList& globalTokens);
        void ParseKeywordExpression(TokenList& localTokens, TokenList& globalTokens);
        void ParseArithmeticOperation(TokenList& localTokens, TokenList& globalTokens);
    };

}

#endif