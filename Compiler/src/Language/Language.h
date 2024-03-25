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
#define KeywordIndex_gameobject    3
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
     * @brief A vector containing all special characters
     */
    extern std::vector<SpecialCharacter> SpecialCharacters;

    /**
     * @brief Checks if the given token is an internal type
     * 
     * @param token The token that will be checked
     * @return `true` if the token is an internal type
     * @return `false` otherwise
     */
    bool IsInternalType(SourceToken& token);
}

#endif