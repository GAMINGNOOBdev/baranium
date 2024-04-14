#ifndef __SOURCETOKEN_H_
#define __SOURCETOKEN_H_ 1

#include <string>

struct SourceToken
{
    enum class Type
    {
        // language stuff

        Invalid,
        Number,
        Keyword,
        Text,

        // custom language stuff
        Null,
        Field,

        // language capabilities/functionalities

        Dot,
        Colon,
        Comma,
        ParenthesisOpen,
        ParenthesisClose,
        BracketOpen,
        BracketClose,
        CurlyBracketOpen,
        CurlyBracketClose,
        ExclamationPoint,
        Semicolon,

        // arithmetics

        Plus,
        Minus,
        Times,
        Divided,
        Modulo,
        And,
        Or,
        Tilde,
        Not, // "^"
        EqualSign,
        
        // strings
        
        Quote,
        DoubleQuote,

        // comparisons

        LessThan,
        GreaterThan,

        // end of the instructed line

        EndOfInstruction = Semicolon,
    } mType;

    std::string Contents;
    int KeywordIndex;
    int LineNumber;

    static SourceToken empty;

    bool operator==(SourceToken& other);
    bool operator!=(SourceToken& other);
};

/**
 * @brief Get the string representation of a token type
 * 
 * @param type The token type
 * @return The string representation of a token type
 */
const char* SourceTokenTypeToString(SourceToken::Type type);

#endif