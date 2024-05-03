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

        case SourceToken::Type::Asterisk:
            return "Asterisk";

        case SourceToken::Type::Slash:
            return "Slash";

        case SourceToken::Type::Modulo:
            return "Modulo";

        case SourceToken::Type::And:
            return "And";

        case SourceToken::Type::Or:
            return "Or";

        case SourceToken::Type::Tilde:
            return "Tilde";

        case SourceToken::Type::Caret:
            return "Caret";

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

SourceTokenIterator::SourceTokenIterator()
    : mTokens(), mIndex(0)
{
}

SourceTokenIterator::SourceTokenIterator(SourceTokenList& list)
    : mTokens(), mIndex(0)
{
    Push(list);
}

void SourceTokenIterator::Clear()
{
    mTokens.clear();
    mIndex = 0;
}

SourceTokenList& SourceTokenIterator::GetTokens()
{
    return mTokens;
}

size_t SourceTokenIterator::GetIndex()
{
    return mIndex;
}

bool SourceTokenIterator::EndOfList()
{
    return mIndex >= mTokens.size();
}

void SourceTokenIterator::Push(SourceToken& token)
{
    mTokens.push_back(token);
}

void SourceTokenIterator::Push(SourceTokenList& list)
{
    mTokens.insert(mTokens.end(), list.begin(), list.end());
}

void SourceTokenIterator::Push(SourceTokenIterator& iterator)
{
    mTokens.insert(mTokens.end(), iterator.mTokens.begin(), iterator.mTokens.end());
}

void SourceTokenIterator::Pop()
{
    mTokens.pop_back();
}

const SourceToken& SourceTokenIterator::Current()
{
    if (mIndex-1 < 0)
        return mTokens.at(mIndex);

    if (mIndex > mTokens.size())
        return SourceToken();

    return mTokens.at(mIndex-1);
}

const SourceToken& SourceTokenIterator::Next()
{
    if (mIndex+1 > mTokens.size())
        return SourceToken();

    auto& token = mTokens.at(mIndex);
    mIndex++;
    return token;
}

const SourceToken& SourceTokenIterator::Peek()
{
    if (EndOfList())
        return SourceToken();

    return mTokens.at(mIndex);
}

bool SourceTokenIterator::NextMatches(SourceToken::Type type)
{
    auto& token = Peek();
    if (token.mType != type)
        return false;

    Next();
    return true;
}
