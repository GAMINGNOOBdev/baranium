#include "Language.h"
#include <algorithm>

using namespace Language;

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

    // loops
    Keyword{"do",           SourceToken::Type::Keyword},
    Keyword{"for",          SourceToken::Type::Keyword},
    Keyword{"while",        SourceToken::Type::Keyword},
};

std::vector<SpecialOperator> Language::SpecialOperators = {
    SpecialOperator{"==",   SourceToken::Type::EqualTo},
    SpecialOperator{"!=",   SourceToken::Type::NotEqual},
    SpecialOperator{"<=",   SourceToken::Type::LessEqual},
    SpecialOperator{">=",   SourceToken::Type::GreaterEqual},
    SpecialOperator{"%=",   SourceToken::Type::ModEqual},
    SpecialOperator{"/=",   SourceToken::Type::DivEqual},
    SpecialOperator{"*=",   SourceToken::Type::MulEqual},
    SpecialOperator{"-=",   SourceToken::Type::MinusEqual},
    SpecialOperator{"+=",   SourceToken::Type::PlusEqual},
    SpecialOperator{"&=",   SourceToken::Type::AndEqual},
    SpecialOperator{"|=",   SourceToken::Type::OrEqual},
    SpecialOperator{"^=",   SourceToken::Type::XorEqual},
    SpecialOperator{"&&",   SourceToken::Type::AndAnd},
    SpecialOperator{"||",   SourceToken::Type::OrOr},
    SpecialOperator{"--",   SourceToken::Type::MinusMinus},
    SpecialOperator{"++",   SourceToken::Type::PlusPlus},
};

std::vector<SpecialCharacter> Language::SpecialCharacters = {
    SpecialCharacter{'+',   SourceToken::Type::Plus},
    SpecialCharacter{'-',   SourceToken::Type::Minus},
    SpecialCharacter{'*',   SourceToken::Type::Times},
    SpecialCharacter{'/',   SourceToken::Type::Divided},
    SpecialCharacter{'%',   SourceToken::Type::Modulo},
    SpecialCharacter{'&',   SourceToken::Type::And},
    SpecialCharacter{'|',   SourceToken::Type::Or},
    SpecialCharacter{'~',   SourceToken::Type::Tilde},
    SpecialCharacter{'^',   SourceToken::Type::Not},
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

std::vector<SpecialCharacter> Language::SpecialOperationCharacters = {
    SpecialCharacter{'+',   SourceToken::Type::Plus},
    SpecialCharacter{'-',   SourceToken::Type::Minus},
    SpecialCharacter{'*',   SourceToken::Type::Times},
    SpecialCharacter{'/',   SourceToken::Type::Divided},
    SpecialCharacter{'%',   SourceToken::Type::Modulo},
    SpecialCharacter{'&',   SourceToken::Type::And},
    SpecialCharacter{'|',   SourceToken::Type::Or},
    SpecialCharacter{'~',   SourceToken::Type::Tilde},
    SpecialCharacter{'^',   SourceToken::Type::Not},
    SpecialCharacter{'=',   SourceToken::Type::EqualSign},
    SpecialCharacter{'<',   SourceToken::Type::LessThan},
    SpecialCharacter{'>',   SourceToken::Type::GreaterThan},
    SpecialCharacter{'!',   SourceToken::Type::ExclamationPoint},
    SpecialCharacter{'(',   SourceToken::Type::ParenthesisOpen},
    SpecialCharacter{')',   SourceToken::Type::ParenthesisClose},
};

bool Language::IsInternalType(SourceToken& token)
{
    return token.KeywordIndex >= KeywordIndex_gameobject && token.KeywordIndex <= KeywordIndex_uint;
}

int Language::IsKeyword(std::string string)
{
    auto iterator = std::find_if(Language::Keywords.begin(), Language::Keywords.end(),
        [string](Language::Keyword& a)
        {
            return a.Name == string;
        }
    );
    if (iterator == Language::Keywords.end())
        return -1;

    return iterator - Language::Keywords.begin();
}

int Language::IsSpecialOperator(char a, char b)
{
    auto iterator = std::find_if(Language::SpecialOperators.begin(), Language::SpecialOperators.end(),
        [a,b](Language::SpecialOperator& op)
        {
            return op.Operator == std::string({a,b});
        }
    );
    if (iterator == Language::SpecialOperators.end())
        return -1;

    return iterator - Language::SpecialOperators.begin();
}

int Language::IsSpecialChar(char c)
{
    auto iterator = std::find_if(Language::SpecialCharacters.begin(), Language::SpecialCharacters.end(),
        [c](Language::SpecialCharacter& a)
        {
            return a.Character == c;
        }
    );
    if (iterator == Language::SpecialCharacters.end())
        return -1;

    return iterator - Language::SpecialCharacters.begin();
}