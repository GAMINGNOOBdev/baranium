#include "Token.h"

namespace Language
{

    /**
     * @brief Get the string representation of a token type
     * 
     * @param type The token type that will be "stringified"
     * 
     * @return The string representation of `type`
     */
    const char* TokenTypeToString(TokenType type)
    {
        switch (type)
        {
            default:
            case TokenType::Invalid:
                return "Invalid";

            case TokenType::Function:
                return "Function";

            case TokenType::Field:
                return "Field";

            case TokenType::Variable:
                return "Variable";
            
            case TokenType::Expression:
                return "Expression";
            
            case TokenType::IfElseStatement:
                return "IfElseStatement";
        }
    }

    /**
     * @brief Construct a new `Token` object
     */
    Token::Token()
    {
        mName = "";
        mTokenType = TokenType::Invalid;
    }

    /**
     * @return The string representation of this `Token`
     */
    std::string Token::ToString()
    {
        return std::string("Token{ Name{").append(mName).append("} Type{").append(TokenTypeToString(mTokenType)).append("} }");
    }

}