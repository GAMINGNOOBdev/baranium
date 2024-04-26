#ifndef __SOURCETOKEN_H_
#define __SOURCETOKEN_H_ 1

#include <string>
#include <vector>

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

        // Partially parsed operations
        EqualTo,
        NotEqual,
        LessEqual,
        GreaterEqual,
        ModEqual,
        DivEqual,
        MulEqual,
        MinusEqual,
        PlusEqual,
        AndEqual,
        OrEqual,
        XorEqual,
        AndAnd,
        OrOr,
        MinusMinus,
        PlusPlus,

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

// shortcut for writing convenience
using SourceTokenList = std::vector<SourceToken>;

struct SourceTokenIterator
{
    SourceTokenIterator();
    SourceTokenIterator(SourceTokenList& list);

    void Clear();

    SourceTokenList& GetTokens();
    size_t GetIndex();

    bool EndOfList();

    void Push(SourceToken& token);
    void Push(SourceTokenList& list);
    void Push(SourceTokenIterator& iterator);
    void Pop();

    SourceToken& Next();
    SourceToken& Peek();

private:
    SourceTokenList mTokens;
    size_t mIndex;
};

#endif