#include "IfElseStatement.h"
#include "../TokenParser.h"
#include "../StringUtil.h"
#include "../Logging.h"

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
        int index = 0;

        mTokens.insert(mTokens.begin(), localTokens.begin(), localTokens.end());

        for (; index+1 < mInnerTokens.size(); index++)
        {
            auto& token = mInnerTokens.at(index);

            if (token.KeywordIndex == KeywordIndex_define)
            {
                TokenParser::ReadFunction(index, token, mInnerTokens, mTokens, globalTokens);
                continue;
            }

            if (token.KeywordIndex == KeywordIndex_if)
            {
                TokenParser::ReadIfStatement(index, token, mInnerTokens, mTokens, globalTokens);
                continue;
            }

            if (token.KeywordIndex == KeywordIndex_else)
            {
                Logging::Log(stringf("Line %d: missing `if` for `else` statement", token.LineNumber));
                Logging::Dispose();
                exit(-1);
            }

            if (token.mType == SourceToken::Type::Field)
            {
                TokenParser::ReadField(index, token, mInnerTokens, mTokens, globalTokens);
                continue;
            }

            if (Language::IsInternalType(token))
            {
                TokenParser::ReadVariable(index, token, mInnerTokens, mTokens, globalTokens);
                continue;
            }

            TokenParser::ReadExpression(index, token, mInnerTokens, mTokens, globalTokens);
        }

        mTokens.erase(mTokens.begin(), mTokens.begin()+localTokens.size());
    }

    /**
     * @returns The string representation of this `Expression`
     */
    std::string IfElseStatement::ToString()
    {
        return std::string(stringf("IfElseStatement{ HasElseStatement{%s} }", HasElseStatement ? "true" : "false"));
    }

}