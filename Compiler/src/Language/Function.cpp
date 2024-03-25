#include "../TokenParser.h"
#include "../StringUtil.h"
#include "../Logging.h"
#include "Function.h"

namespace Language
{

    /**
     * @brief Construct a new `Function` object
     */
    Function::Function()
        : Token()
    {
        mTokenType = TokenType::Function;
        ReturnType = VariableType::Void;
        ReturnValue = "";
        ReturnVariableName = "";
    }

    /**
     * @brief Parse the current inner tokens
     */
    void Function::ParseTokens(TokenList& globalTokens)
    {
        int index = 0;

        mTokens.insert(mTokens.begin(), mParameters.begin(), mParameters.end());

        for (; index < mInnerTokens.size() && !FunctionReturnRequested(); index++)
        {
            auto& token = mInnerTokens.at(index);

            if (token.KeywordIndex == KeywordIndex_define)
            {
                Logging::Log(stringf("Line %d: Invalid function syntax: function inside function", token.LineNumber), Logging::Level::Error);
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
                Logging::Log(stringf("Line %d: Invalid function syntax: fields inside function not allowed", token.LineNumber), Logging::Level::Error);
                Logging::Dispose();
                exit(-1);
            }

            if (Language::IsInternalType(token))
            {
                TokenParser::ReadVariable(index, token, mInnerTokens, mTokens, mTokens);
                continue;
            }

            TokenParser::ReadExpression(index, token, mInnerTokens, mTokens, globalTokens);
        }
        // only needed here because the return statement can also be the last token in the function
        FunctionReturnRequested();

        mTokens.erase(mTokens.begin(), mTokens.begin()+mParameters.size());
    }

    /**
     * @returns The string representation of this `Function`
     */
    std::string Function::ToString()
    {
        return std::string(stringf("Function{ Name{'%s'} ParameterCount{%ld} InnerTokenCount{%ld} TokenCount{%ld} }",
                                   mName.c_str(), mParameters.size(), mInnerTokens.size(), mTokens.size() ));
    }

    bool Function::FunctionReturnRequested()
    {
        if (mTokens.empty())
            return false;

        if (mTokens.at(mTokens.size()-1)->mTokenType != TokenType::Expression)
            return false;

        auto expression = std::static_pointer_cast<Language::Expression, Language::Token>(mTokens.at(mTokens.size()-1));
        if (expression->Type == Language::ExpressionType::ReturnStatement)
        {
            ReturnType = VariableType(expression->ReturnType);
            ReturnValue = std::string(expression->ReturnValue);
            ReturnVariableName = std::string(expression->ReturnVariableName);

            return true;
        }

        return false;
    }

}