#include <baranium/compiler/language/language.h>
#include <baranium/compiler/source_token.h>
#include <string.h>

baranium_keyword baranium_keywords[] = {
    // the "field", a custom type that can be set outside of this environment
    {"field",         BARANIUM_SOURCE_TOKEN_TYPE_FIELD},
    {"attached",      BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},

    // the null type
    {"null",          BARANIUM_SOURCE_TOKEN_TYPE_NULL},

    // types
    {"object",        BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"string",        BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"float",         BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"bool",          BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"int32",         BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"uint32",        BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"double",        BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"int8",         BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"uint8",        BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"int16",        BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"uint16",       BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"int64",        BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"uint64",       BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"void",         BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},

    // normal language stuff (function definitions)
    {"define",       BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"return",       BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"true",         BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"false",        BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"if",           BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"else",         BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},

    // object management related stuff
    {"instantiate",  BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"delete",       BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},

    // script related stuff
    {"attach",       BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"detach",       BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},

    // loops
    {"do",           BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"for",          BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"while",        BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"break",        BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},
    {"continue",     BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},

    {"struct",       BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD},

    {0,BARANIUM_SOURCE_TOKEN_TYPE_INVALID}
};

baranium_special_operator baranium_special_operators[] = {
    {"==",   BARANIUM_SOURCE_TOKEN_TYPE_EQUALTO},
    {"!=",   BARANIUM_SOURCE_TOKEN_TYPE_NOTEQUAL},
    {"<=",   BARANIUM_SOURCE_TOKEN_TYPE_LESSEQUAL},
    {">=",   BARANIUM_SOURCE_TOKEN_TYPE_GREATEREQUAL},
    {"%=",   BARANIUM_SOURCE_TOKEN_TYPE_MODEQUAL},
    {"/=",   BARANIUM_SOURCE_TOKEN_TYPE_DIVEQUAL},
    {"*=",   BARANIUM_SOURCE_TOKEN_TYPE_MULEQUAL},
    {"-=",   BARANIUM_SOURCE_TOKEN_TYPE_MINUSEQUAL},
    {"+=",   BARANIUM_SOURCE_TOKEN_TYPE_PLUSEQUAL},
    {"&=",   BARANIUM_SOURCE_TOKEN_TYPE_ANDEQUAL},
    {"|=",   BARANIUM_SOURCE_TOKEN_TYPE_OREQUAL},
    {"^=",   BARANIUM_SOURCE_TOKEN_TYPE_XOREQUAL},
    {"%%",   BARANIUM_SOURCE_TOKEN_TYPE_MODMOD},
    {"&&",   BARANIUM_SOURCE_TOKEN_TYPE_ANDAND},
    {"||",   BARANIUM_SOURCE_TOKEN_TYPE_OROR},
    {"--",   BARANIUM_SOURCE_TOKEN_TYPE_MINUSMINUS},
    {"++",   BARANIUM_SOURCE_TOKEN_TYPE_PLUSPLUS},
    {"<<",   BARANIUM_SOURCE_TOKEN_TYPE_BITSHIFTLEFT},
    {">>",   BARANIUM_SOURCE_TOKEN_TYPE_BITSHIFTRIGHT},
    {0,BARANIUM_SOURCE_TOKEN_TYPE_INVALID}
};

baranium_special_character baranium_special_characters[] = {
    {"+",   BARANIUM_SOURCE_TOKEN_TYPE_PLUS},
    {"-",   BARANIUM_SOURCE_TOKEN_TYPE_MINUS},
    {"*",   BARANIUM_SOURCE_TOKEN_TYPE_ASTERISK},
    {"/",   BARANIUM_SOURCE_TOKEN_TYPE_SLASH},
    {"%",   BARANIUM_SOURCE_TOKEN_TYPE_MODULO},
    {"&",   BARANIUM_SOURCE_TOKEN_TYPE_AND},
    {"|",   BARANIUM_SOURCE_TOKEN_TYPE_OR},
    {"~",   BARANIUM_SOURCE_TOKEN_TYPE_TILDE},
    {"^",   BARANIUM_SOURCE_TOKEN_TYPE_CARET},
    {"(",   BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN},
    {")",   BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISCLOSE},
    {"=",   BARANIUM_SOURCE_TOKEN_TYPE_EQUALSIGN},

    {"[",   BARANIUM_SOURCE_TOKEN_TYPE_BRACKETOPEN},
    {"]",   BARANIUM_SOURCE_TOKEN_TYPE_BRACKETCLOSE},
    {"{",   BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETOPEN},
    {"}",   BARANIUM_SOURCE_TOKEN_TYPE_CURLYBRACKETCLOSE},

    {".",   BARANIUM_SOURCE_TOKEN_TYPE_DOT},
    {":",   BARANIUM_SOURCE_TOKEN_TYPE_COLON},
    {",",   BARANIUM_SOURCE_TOKEN_TYPE_COMMA},

    {"<",   BARANIUM_SOURCE_TOKEN_TYPE_LESSTHAN},
    {">",   BARANIUM_SOURCE_TOKEN_TYPE_GREATERTHAN},

    {"\"",   BARANIUM_SOURCE_TOKEN_TYPE_DOUBLEQUOTE},
    {"\'",  BARANIUM_SOURCE_TOKEN_TYPE_QUOTE},
    {"!",   BARANIUM_SOURCE_TOKEN_TYPE_EXCLAMATIONPOINT},
    {";",   BARANIUM_SOURCE_TOKEN_TYPE_SEMICOLON},
    {0,BARANIUM_SOURCE_TOKEN_TYPE_INVALID}
};

baranium_special_character baranium_special_operation_characters[] = {
    {"+", BARANIUM_SOURCE_TOKEN_TYPE_PLUS},
    {"-", BARANIUM_SOURCE_TOKEN_TYPE_MINUS},
    {"*", BARANIUM_SOURCE_TOKEN_TYPE_ASTERISK},
    {"/", BARANIUM_SOURCE_TOKEN_TYPE_SLASH},
    {"%", BARANIUM_SOURCE_TOKEN_TYPE_MODULO},
    {"&", BARANIUM_SOURCE_TOKEN_TYPE_AND},
    {"|", BARANIUM_SOURCE_TOKEN_TYPE_OR},
    {"~", BARANIUM_SOURCE_TOKEN_TYPE_TILDE},
    {"^", BARANIUM_SOURCE_TOKEN_TYPE_CARET},
    {"=", BARANIUM_SOURCE_TOKEN_TYPE_EQUALSIGN},
    {"<", BARANIUM_SOURCE_TOKEN_TYPE_LESSTHAN},
    {">", BARANIUM_SOURCE_TOKEN_TYPE_GREATERTHAN},
    {"!", BARANIUM_SOURCE_TOKEN_TYPE_EXCLAMATIONPOINT},
    {"(", BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN},
    {")", BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISCLOSE},
    {0,BARANIUM_SOURCE_TOKEN_TYPE_INVALID},
};

uint8_t baranium_is_internal_type(baranium_source_token token)
{
    if (token.special_index >= BARANIUM_KEYWORDS_TYPES_START && token.special_index <= BARANIUM_KEYWORDS_TYPES_END)
        return strcmp(token.contents, baranium_keywords[token.special_index].name) == 0;

    return 0;
}

int baranium_is_keyword(const char* string)
{
    for (int i = 0; baranium_keywords[i].name != NULL; i++)
        if (strcmp(string, baranium_keywords[i].name) == 0)
            return i;

    return -1;
}

int baranium_is_special_operator(char a, char b)
{
    char str[] = {a,b,0};

    for (int i = 0; baranium_special_operators[i].name != NULL; i++)
        if (strcmp(str, baranium_special_operators[i].name) == 0)
            return i;

    return -1;
}

int baranium_is_special_char(char c)
{
    for (int i = 0; baranium_special_characters[i].name != 0; i++)
        if (baranium_special_characters[i].name[0] == c)
            return i;

    return -1;
}
