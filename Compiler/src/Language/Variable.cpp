#include "../StringUtil.h"
#include "Variable.h"

namespace Language
{

    /**
     * @brief Get the string representation of a variable type
     * 
     * @param type The variable type that will be "stringified"
     * 
     * @return The string representation of `type`
     */
    const char* VariableTypeToString(VariableType type)
    {
        switch (type)
        {
            default:
            case VariableType::Invalid:
                return "Invalid";
            
            case VariableType::Void:
                return "Void";

            case VariableType::GameObject:
                return "GameObject";

            case VariableType::String:
                return "String";

            case VariableType::Float:
                return "Float";

            case VariableType::Bool:
                return "Bool";

            case VariableType::Int:
                return "Int";

            case VariableType::Uint:
                return "Uint";
        }
    }

    /**
     * @brief Get the variable type from a single token
     * 
     * @param token The token that will be converted into the variable type
     * @return The variable type
     */
    VariableType Variable::TypeFromToken(SourceToken& token)
    {
        if (!IsInternalType(token))
        {
            return VariableType::Invalid;
        }

        return (VariableType)( (int)VariableType::GameObject + token.KeywordIndex - (KEYWORDS_TYPES_START) );
    }

    /**
     * @brief Predict the variable type from a list of tokens
     * 
     * @param tokens The list of tokens that represent the variable type
     * @return The predicted variable type 
     */
    VariableType Variable::PredictType(SourceTokenList& tokens)
    {
        int tokenIndex = 0;

        auto& firstToken = tokens.at(tokenIndex++);

        if (firstToken.KeywordIndex == KeywordIndex_true || firstToken.KeywordIndex == KeywordIndex_false)
            return VariableType::Bool;

        if (firstToken.mType == SourceToken::Type::DoubleQuote)
            return VariableType::String;

        if (firstToken.mType == SourceToken::Type::Minus || firstToken.mType == SourceToken::Type::Plus)
        {
            if (tokenIndex >= tokens.size())
                return VariableType::Invalid;

            auto& secondToken = tokens.at(tokenIndex++);
            if (secondToken.mType == SourceToken::Type::Number)
            {
                if (tokenIndex+1 < tokens.size())
                {
                    auto& thirdToken = tokens.at(tokenIndex++);
                    if (thirdToken.mType == SourceToken::Type::Dot)
                        return VariableType::Float;
                }
                return firstToken.mType == SourceToken::Type::Minus ? VariableType::Int : VariableType::Uint;
            }
        }

        if (firstToken.mType == SourceToken::Type::Number)
        {
            if (tokenIndex >= tokens.size())
                return VariableType::Uint;
            
            auto& secondToken = tokens.at(tokenIndex++);
            if (secondToken.mType == SourceToken::Type::Dot)
                return VariableType::Float;
        }

        if (firstToken.mType == SourceToken::Type::Null)
            return VariableType::Void;

        return VariableType::Invalid;
    }

    /**
     * @brief Construct a new `Variable` object
     */
    Variable::Variable()
        : Token()
    {
        mTokenType = TokenType::Variable;
    }

    /**
     * @returns The string representation of this `Variable`
     */
    std::string Variable::ToString()
    {
        return std::string(stringf("Variable{ Name{'%s'} Type{%s} Value{'%s'} }", mName.c_str(), VariableTypeToString(Type), Value.c_str()));
    }

}