#ifndef __LANGUAGE__IFELSESTATEMENT_H_
#define __LANGUAGE__IFELSESTATEMENT_H_ 1

#include "Expression.h"
#include "Language.h"
#include <stdint.h>
#include "Token.h"
#include <string>
#include <memory>
#include <vector>
#include <list>

using SourceTokenList = std::vector<SourceToken>;
using TokenList = std::vector<std::shared_ptr<Language::Token>>;

namespace Language
{

    struct IfElseStatement : public Token
    {
        Expression Condition;
        SourceTokenList mInnerTokens;
        TokenList mTokens;
        std::list<IfElseStatement> ChainedStatements;
        bool HasElseStatement;

        /**
         * @brief Construct a new `IfElseStatement` object
         */
        IfElseStatement();

        /**
         * @brief Identify the type of this expression
         */
        void ParseTokens(TokenList& localTokens, TokenList& globalTokens = Language::EmptyTokenList);

        /**
         * @returns The string representation of this `Expression`
         */
        std::string ToString() override;
    };

}

#endif