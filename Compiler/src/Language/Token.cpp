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

            case TokenType::DoWhileLoop:
                return "DoWhileLoop";

            case TokenType::WhileLoop:
                return "WhileLoop";

            case TokenType::ForLoop:
                return "ForLoop";
        }
    }

    index_t Token::TokenID = 0;

    /**
     * @brief Construct a new `Token` object
     */
    Token::Token()
    {
        ID = -1;
        mName = "";
        mTokenType = TokenType::Invalid;
    }

    /**
     * @brief small function that assigns an internal ID
     */
    void Token::AssignID()
    {
        ID = TokenID;
        TokenID++;
    }

}