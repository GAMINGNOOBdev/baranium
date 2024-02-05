#include "IfElseStatement.h"
#include "../StringUtil.h"

namespace Language
{

    /**
     * @brief Construct a new `IfElseStatement` object
     */
    IfElseStatement::IfElseStatement()
        : HasElseStatement(false)
    {
        mTokenType = TokenType::IfElseStatement;
        mName = "";
    }

    /**
     * @brief Identify the type of this expression
     */
    void IfElseStatement::ParseTokens(TokenList& localTokens, TokenList& globalTokens)
    {
        //
    }

    /**
     * @returns The string representation of this `Expression`
     */
    std::string IfElseStatement::ToString()
    {
        return std::string(stringf("IfElseStatement{ HasElseStatement{%s} }", HasElseStatement ? "true" : "false"));
    }

}