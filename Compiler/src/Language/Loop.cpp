#include "../TokenParser.h"
#include "../StringUtil.h"
#include "../Logging.h"
#include "Loop.h"

namespace Language
{

    Loop::Loop()
        : Token()
    {
        mTokenType = TokenType::Invalid;
        DoWhile = false;
        While = false;
        mName = "";
    }

    Loop::Loop(TokenType loopType)
        : Token()
    {
        mTokenType = loopType;
        DoWhile = false;
        While = false;
        mName = "";
    }

    void Loop::ParseTokens(TokenList& localTokens, TokenList& globalTokens)
    {
        int index = 0;

        mTokens.insert(mTokens.begin(), localTokens.begin(), localTokens.end());
        if (StartVariable)
            mTokens.push_back(StartVariable);

        Condition.Identify(mTokens, globalTokens);
        Iteration.Identify(mTokens, globalTokens);

        for (; index < mInnerTokens.size(); index++)
        {
            auto& token = mInnerTokens.at(index);

            if (token.KeywordIndex == KeywordIndex_define)
                Logging::LogErrorExit(stringf("Line %d: Invalid syntax: function definition inside a loop", token.LineNumber));

            if (token.KeywordIndex >= KeywordIndex_do && token.KeywordIndex <= KeywordIndex_while)
            {
                TokenParser::ReadLoop(index, token, mInnerTokens, mTokens, mTokens);
                continue;
            }

            if (token.KeywordIndex == KeywordIndex_if)
            {
                TokenParser::ReadIfStatement(index, token, mInnerTokens, mTokens, mTokens);
                continue;
            }

            if (token.KeywordIndex == KeywordIndex_else)
                Logging::LogErrorExit(stringf("Line %d: missing `if` for `else` statement", token.LineNumber));

            if (token.mType == SourceToken::Type::Field)
                Logging::LogErrorExit(stringf("Line %d: Invalid syntax: fields are not allowed outside of the global scope", token.LineNumber));

            if (Language::IsInternalType(token))
                Logging::LogErrorExit(stringf("Line %d: Invalid syntax: variables are not allowed inside of loops", token.LineNumber));

            TokenParser::ReadExpression(index, token, mInnerTokens, mTokens, globalTokens);
        }

        mTokens.erase(mTokens.begin(), mTokens.begin()+localTokens.size());
        if (StartVariable)
            mTokens.erase(mTokens.begin(), mTokens.begin()+1);
    }

    void LoopTypeFromToken(SourceToken& loopIdentifier, std::shared_ptr<Loop> loop)
    {
        loop->DoWhile = (loopIdentifier.KeywordIndex == KeywordIndex_do);
        loop->While =   (loopIdentifier.KeywordIndex == KeywordIndex_while || loopIdentifier.KeywordIndex == KeywordIndex_do);
    }

}
