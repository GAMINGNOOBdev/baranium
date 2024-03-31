#include "../TokenParser.h"
#include "../StringUtil.h"
#include "../Logging.h"
#include "Loop.h"

namespace Language
{

    /**
     * @brief Construct a new `Loop` object
     */
    Loop::Loop()
        : Token()
    {
        mTokenType = TokenType::Invalid;
        DoWhile = false;
        While = false;
        mName = "";
    }

    /**
     * @brief Construct a new `Loop` object
     * 
     * @param loopType Token type for the type of this loop
     */
    Loop::Loop(TokenType loopType)
        : Token()
    {
        mTokenType = loopType;
        DoWhile = false;
        While = false;
        mName = "";
    }

    /**
     * @brief Identify and parse inner tokens
     */
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
            {
                Logging::Log(stringf("Line %d: Invalid syntax: function definition inside a loop", token.LineNumber), Logging::Level::Error);
                Logging::Dispose();
                exit(-1);
            }

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
            {
                Logging::Log(stringf("Line %d: missing `if` for `else` statement", token.LineNumber));
                Logging::Dispose();
                exit(-1);
            }

            if (token.mType == SourceToken::Type::Field)
            {
                Logging::Log(stringf("Line %d: Invalid syntax: fields are not allowed outside of the global scope", token.LineNumber), Logging::Level::Error);
                Logging::Dispose();
                exit(-1);
            }

            if (Language::IsInternalType(token))
            {
                Logging::Log(stringf("Line %d: Invalid syntax: variables are not allowed inside of loops", token.LineNumber), Logging::Level::Error);
                Logging::Dispose();
                exit(-1);
            }

            TokenParser::ReadExpression(index, token, mInnerTokens, mTokens, globalTokens);
        }

        mTokens.erase(mTokens.begin(), mTokens.begin()+localTokens.size());
        if (StartVariable)
            mTokens.erase(mTokens.begin(), mTokens.begin()+1);
    }

    /**
     * @brief Set the type of a loop by identifying it from a token
     * 
     * @param loopIdentifier The token that will be used to identify the loop type
     * @param loop The loop object that will get the identified type
     */
    void LoopTypeFromToken(SourceToken& loopIdentifier, std::shared_ptr<Loop> loop)
    {
        loop->DoWhile = (loopIdentifier.KeywordIndex == KeywordIndex_do);
        loop->While =   (loopIdentifier.KeywordIndex == KeywordIndex_while || loopIdentifier.KeywordIndex == KeywordIndex_do);
    }

}