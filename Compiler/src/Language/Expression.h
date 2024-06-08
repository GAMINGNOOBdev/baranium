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
        None,
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
        int LineNumber;
        ExpressionType Type;
        AbstractSyntaxTree mAST;
        std::string ReturnValue;
        VariableType ReturnType;
        SourceTokenList mInnerTokens;
        std::string ReturnVariableName;
        std::shared_ptr<Expression> ReturnExpression;

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
        void ParseReturnStatement(SourceTokenList& tokens, TokenList& localTokens, TokenList& globalTokens);
    };

}

#endif
