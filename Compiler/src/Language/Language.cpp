#include "Language.h"

using namespace Language;

/**
 * @brief A vector containing all possible keywords
 */
std::vector<Keyword> Language::Keywords = {
    // the "field", a custom type that can be set outside of this environment
    Keyword{"field",        SourceToken::Type::Field},
    Keyword{"attached",     SourceToken::Type::Keyword},

    // the null type
    Keyword{"null",         SourceToken::Type::Null},

    // types
    Keyword{"gameobject",   SourceToken::Type::Keyword},
    Keyword{"string",       SourceToken::Type::Keyword},
    Keyword{"float",        SourceToken::Type::Keyword},
    Keyword{"bool",         SourceToken::Type::Keyword},
    Keyword{"int",          SourceToken::Type::Keyword},
    Keyword{"uint",         SourceToken::Type::Keyword},

    // normal language stuff (function definitions)
    Keyword{"define",       SourceToken::Type::Keyword},
    Keyword{"return",       SourceToken::Type::Keyword},
    Keyword{"true",         SourceToken::Type::Keyword},
    Keyword{"false",        SourceToken::Type::Keyword},
    Keyword{"if",           SourceToken::Type::Keyword},
    Keyword{"else",         SourceToken::Type::Keyword},

    // gameobject management related stuff
    Keyword{"instantiate",  SourceToken::Type::Keyword},
    Keyword{"delete",       SourceToken::Type::Keyword},

    // script related stuff
    Keyword{"attach",       SourceToken::Type::Keyword},
    Keyword{"detach",       SourceToken::Type::Keyword},
};

/**
 * @brief A vector containing all special characters
 */
std::vector<SpecialCharacter> Language::SpecialCharacters = {
    SpecialCharacter{'+',   SourceToken::Type::Plus},
    SpecialCharacter{'-',   SourceToken::Type::Minus},
    SpecialCharacter{'*',   SourceToken::Type::Times},
    SpecialCharacter{'/',   SourceToken::Type::Divided},
    SpecialCharacter{'(',   SourceToken::Type::ParenthesisOpen},
    SpecialCharacter{')',   SourceToken::Type::ParenthesisClose},
    SpecialCharacter{'=',   SourceToken::Type::EqualSign},

    SpecialCharacter{'[',   SourceToken::Type::BracketOpen},
    SpecialCharacter{']',   SourceToken::Type::BracketClose},
    SpecialCharacter{'{',   SourceToken::Type::CurlyBracketOpen},
    SpecialCharacter{'}',   SourceToken::Type::CurlyBracketClose},

    SpecialCharacter{'.',   SourceToken::Type::Dot},
    SpecialCharacter{':',   SourceToken::Type::Colon},
    SpecialCharacter{',',   SourceToken::Type::Comma},

    SpecialCharacter{'<',   SourceToken::Type::LessThan},
    SpecialCharacter{'>',   SourceToken::Type::GreaterThan},

    SpecialCharacter{'"',   SourceToken::Type::DoubleQuote},
    SpecialCharacter{'\'',  SourceToken::Type::Quote},
    SpecialCharacter{'!',   SourceToken::Type::ExclamationPoint},
    SpecialCharacter{';',   SourceToken::Type::Semicolon},
};

/**
 * @brief Checks if the given token is an internal type
 * 
 * @param token The token that will be checked
 * @return `true` if the token is an internal type
 * @return `false` otherwise
 */
bool Language::IsInternalType(SourceToken& token)
{
    return token.KeywordIndex >= KeywordIndex_gameobject && token.KeywordIndex <= KeywordIndex_uint;
}