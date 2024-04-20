#include "SourceToken.h"
#include "StringUtil.h"

SourceToken SourceToken::empty = SourceToken();

bool SourceToken::operator==(SourceToken& other)
{
    return other.Contents == Contents && other.KeywordIndex == KeywordIndex && other.LineNumber == LineNumber && other.mType == mType;
}

bool SourceToken::operator!=(SourceToken& other)
{
    return !operator==(other);
}

const char* SourceTokenTypeToString(SourceToken::Type type)
{
    switch (type)
    {
        case SourceToken::Type::Invalid:
            return "Invalid";

        case SourceToken::Type::Number:
            return "Number";

        case SourceToken::Type::Keyword:
            return "Keyword";

        case SourceToken::Type::Text:
            return "Text";

        case SourceToken::Type::Null:
            return "Null";

        case SourceToken::Type::Field:
            return "Field";

        case SourceToken::Type::Dot:
            return "Dot";

        case SourceToken::Type::Colon:
            return "Colon";

        case SourceToken::Type::Comma:
            return "Comma";

        case SourceToken::Type::ParenthesisOpen:
            return "ParenthesisOpen";

        case SourceToken::Type::ParenthesisClose:
            return "ParenthesisClose";

        case SourceToken::Type::BracketOpen:
            return "BracketOpen";

        case SourceToken::Type::BracketClose:
            return "BracketClose";

        case SourceToken::Type::CurlyBracketOpen:
            return "CurlyBracketOpen";

        case SourceToken::Type::CurlyBracketClose:
            return "CurlyBracketClose";

        case SourceToken::Type::ExclamationPoint:
            return "ExclamationPoint";

        case SourceToken::Type::Semicolon:
            return "Semicolon";

        case SourceToken::Type::Plus:
            return "Plus";

        case SourceToken::Type::Minus:
            return "Minus";

        case SourceToken::Type::Times:
            return "Times";

        case SourceToken::Type::Divided:
            return "Divided";
        
        case SourceToken::Type::Modulo:
            return "Modulo";

        case SourceToken::Type::And:
            return "And";

        case SourceToken::Type::Or:
            return "Or";

        case SourceToken::Type::Tilde:
            return "Tilde";
        
        case SourceToken::Type::Not:
            return "Not";

        case SourceToken::Type::EqualSign:
            return "EqualSign";

        case SourceToken::Type::EqualTo:
            return "EqualTo";

        case SourceToken::Type::NotEqual:
            return "NotEqual";

        case SourceToken::Type::LessEqual:
            return "LessEqual";

        case SourceToken::Type::GreaterEqual:
            return "GreaterEqual";

        case SourceToken::Type::ModEqual:
            return "ModEqual";

        case SourceToken::Type::DivEqual:
            return "DivEqual";

        case SourceToken::Type::MulEqual:
            return "MulEqual";

        case SourceToken::Type::MinusEqual:
            return "MinusEqual";

        case SourceToken::Type::PlusEqual:
            return "PlusEqual";

        case SourceToken::Type::AndEqual:
            return "AndEqual";

        case SourceToken::Type::OrEqual:
            return "OrEqual";

        case SourceToken::Type::XorEqual:
            return "XorEqual";

        case SourceToken::Type::AndAnd:
            return "AndAnd";

        case SourceToken::Type::OrOr:
            return "OrOr";

        case SourceToken::Type::Quote:
            return "Quote";

        case SourceToken::Type::DoubleQuote:
            return "DoubleQuote";

        case SourceToken::Type::LessThan:
            return "LessThan";

        case SourceToken::Type::GreaterThan:
            return "GreaterThan";
    }

    return "none";
}