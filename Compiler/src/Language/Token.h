#ifndef __LANGUAGE__TYPE_H_
#define __LANGUAGE__TYPE_H_ 1

#include <string>
#include <vector>
#include <memory>

namespace Language
{

    enum class TokenType
    {
        Invalid,
        Field,
        Variable,
        Function,
        Expression,
        IfElseStatement,
        DoWhileLoop,
        WhileLoop,
        ForLoop,
    };

    /**
     * @brief Get the string representation of a token type
     * 
     * @param type The token type that will be "stringified"
     * 
     * @return The string representation of `type`
     */
    const char* TokenTypeToString(TokenType type);

    struct Token
    {
        std::string mName;
        TokenType mTokenType = TokenType::Invalid;

        /**
         * @brief Construct a new `Token` object
         */
        Token();

        /**
         * @return The string representation of this `Token`
         */
        virtual std::string ToString();
    };

    static std::vector<std::shared_ptr<Language::Token>> EmptyTokenList;

}

#endif