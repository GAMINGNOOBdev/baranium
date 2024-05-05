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
        Asterisk,
        Slash,
        Modulo,
        And,
        Or,
        Tilde,
        Caret, // "^"
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
        BitshiftLeft,
        BitshiftRight,

        // strings

        Quote,
        DoubleQuote,

        // comparisons

        LessThan,
        GreaterThan,

        // end of the instructed line

        EndOfInstruction = Semicolon,
    } mType;

    SourceToken();
    SourceToken(const SourceToken& other);

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

/**
 * @brief An "iterator"/helper class for iterating through a list of source tokens
 */
struct SourceTokenIterator
{
    /**
     * @brief Construct a new `SourceTokenIterator` object
     */
    SourceTokenIterator();

    /**
     * @brief Construct a new `SourceTokenIterator` object
     * 
     * @param list The source token list that will be used by this "iterator"
     */
    SourceTokenIterator(SourceTokenList& list);

    /**
     * @brief Clear all current tokens + current index
     */
    void Clear();

    /**
     * @brief Get the current tokens
     */
    SourceTokenList& GetTokens();

    /**
     * @brief Get the current token index
     */
    size_t GetIndex();

    /**
     * @brief Check if we reached the end of the list
     */
    bool EndOfList();

    /**
     * @brief Push a new token to the end of the list
     * 
     * @param token The new token
     */
    void Push(SourceToken& token);

    /**
     * @brief Push a list of tokens to the end of the list
     * 
     * @param list The list that will be appended to the end of the tokens list
     */
    void Push(SourceTokenList& list);

    /**
     * @brief Push/Append another token iterator and it's list to this iterator
     * 
     * @param iterator The other iterator
     */
    void Push(SourceTokenIterator& iterator);

    /**
     * @brief Pop the last added token from the list
     */
    void Pop();

    /**
     * @brief Get the current token
     */
    const SourceToken& Current();

    /**
     * @brief Go to the next token and consume the last one
     */
    const SourceToken& Next();

    /**
     * @brief Get the next token without consuming the current one
     */
    const SourceToken& Peek();

    /**
     * @brief Check if the next tokens type matches the desired type
     */
    bool NextMatches(SourceToken::Type type);

private:
    SourceTokenList mTokens;
    int64_t mIndex;
};

#endif