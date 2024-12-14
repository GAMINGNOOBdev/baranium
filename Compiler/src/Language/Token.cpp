#include <algorithm>
#include "Token.h"

namespace Language
{

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

    Token::Token()
    {
        ID = -1;
        mName = "";
        mTokenType = TokenType::Invalid;
    }

    void Token::AssignID()
    {
        ID = TokenID;

        if (!mName.empty())
        {
            uint64_t identifier = 0;
            for (size_t i = 0; i < mName.size(); i++)
                identifier |= (mName.at(i)-'A') << (i % 8) * 8;

            ID += identifier;
        }

        TokenID++;
    }

}

std::shared_ptr<Language::Token> TokensListContains(std::string name, TokenList& localTokens, TokenList& globalTokens)
{
    auto globalNameIterator = std::find_if(globalTokens.begin(), globalTokens.end(), [name](std::shared_ptr<Language::Token>& token)
    {
        return token->mName == name;
    });
    auto nameIterator = std::find_if(localTokens.begin(), localTokens.end(), [name](std::shared_ptr<Language::Token>& token)
    {
        return token->mName == name;
    });
    if (nameIterator == localTokens.end() && globalNameIterator == globalTokens.end())
        return nullptr;
    
    if (globalNameIterator != globalTokens.end())
        return *globalNameIterator;

    return *nameIterator;
}
