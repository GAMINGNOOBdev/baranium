#ifndef __LANGUAGE__LANGUAGE_H_
#define __LANGUAGE__LANGUAGE_H_ 1

#include "../SourceToken.h"
#include <string>
#include <vector>

#define KEYWORDS_TYPES_START 3
#define KEYWORDS_TYPES_END 8

#define KeywordIndex_field         0
#define KeywordIndex_attached      1
#define KeywordIndex_null          2
#define KeywordIndex_object        3
#define KeywordIndex_string        4
#define KeywordIndex_float         5
#define KeywordIndex_bool          6
#define KeywordIndex_int           7
#define KeywordIndex_uint          8
#define KeywordIndex_define        9
#define KeywordIndex_return        10
#define KeywordIndex_true          11
#define KeywordIndex_false         12
#define KeywordIndex_if            13
#define KeywordIndex_else          14
#define KeywordIndex_instantiate   15
#define KeywordIndex_delete        16
#define KeywordIndex_attach        17
#define KeywordIndex_detach        18
#define KeywordIndex_do            19
#define KeywordIndex_for           20
#define KeywordIndex_while         21

namespace Language
{

    /**
     * @brief The keyword container that also stores what token type it is
     */
    struct Keyword
    {
        std::string Name;
        SourceToken::Type TokenType;
    };

    /**
     * @brief A container for special operators that store
     *        the token type alongside the operator string itself
     * 
     */
    struct SpecialOperator
    {
        std::string Operator;
        SourceToken::Type TokenType;
    };

    /**
     * @brief A container for special characters that stores
     *        it's token type alongside the character itself
     * 
     */
    struct SpecialCharacter
    {
        char Character;
        SourceToken::Type TokenType;
    };

    /**
     * @brief A vector containing all possible keywords
     */
    extern std::vector<Keyword> Keywords;

    /**
     * @brief A vector containing all special operators
     */
    extern std::vector<SpecialOperator> SpecialOperators;

    /**
     * @brief A vector containing all special characters
     */
    extern std::vector<SpecialCharacter> SpecialCharacters;

    /**
     * @brief A vector containing all special characters for operations
     */
    extern std::vector<SpecialCharacter> SpecialOperationCharacters;

    /**
     * @brief Checks if the given token is an internal type
     * 
     * @param token The token that will be checked
     * @return `true` if the token is an internal type
     * @return `false` otherwise
     */
    bool IsInternalType(SourceToken& token);

    /**
     * @brief Checks if the given string is a keyword
     * 
     * @param string The string that will be checked if it is a keyword
     * 
     * @returns -1 if not a keyword, else it will return the index of it
     */
    int IsKeyword(std::string string);

    /**
     * @brief Checks if the given characters `a` and `b` make up a special operator
     * 
     * @param a The first character of the operator
     * @param b The second character of the operator
     * 
     * @returns -1 if not a special operator, else an index to the special operator in the list
     */
    int IsSpecialOperator(char a, char b);

    /**
     * @brief Checks if the given character `c` is a special character
     * 
     * @param c The character that will be checked
     * 
     * @returns -1 if not a special character, else an index to the special character in the list
     */
    int IsSpecialChar(char c);
}

#endif