#ifndef __LANGUAGE__TYPE_H_
#define __LANGUAGE__TYPE_H_ 1

#include <string>
#include <vector>
#include <memory>

typedef int64_t index_t;

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
        static index_t TokenID;

        index_t ID;
        std::string mName;
        TokenType mTokenType = TokenType::Invalid;

        /**
         * @brief Construct a new `Token` object
         */
        Token();

        /**
         * @brief small function that assigns an internal ID
         */
        void AssignID();
    };

    static std::vector<std::shared_ptr<Language::Token>> EmptyTokenList;

}

// shortcut for writing convenience
using TokenList = std::vector<std::shared_ptr<Language::Token>>;

#endif