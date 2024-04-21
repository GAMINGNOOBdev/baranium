#ifndef __TOKENPARSER_H_
#define __TOKENPARSER_H_

#include "Language/Types.h"
#include "Source.h"
#include <vector>
#include <memory>

/**
 * @brief An object that parses tokens from a source to then create a final binary
 */
struct TokenParser
{
    /**
     * @brief Construct a new `TokenParser`
     */
    TokenParser();

    /**
     * @brief Write the source code tokens into a single json file
     * 
     * @note Executed then the `-d` or `--debug` flag is passed to the main executable
     * 
     * @param name Filename of the json file where the token data will be written to
     */
    void WriteTokensToJson(std::string name);

    /**
     * @brief Parse the incoming tokens
     * 
     * @param tokens Tokens that will be parsed
     */
    void ParseTokens(SourceTokenList& tokens);

    /**
     * @brief Get the parsed token objects
     * 
     * @return The list of parsed tokens
     */
    TokenList& GetTokens();

    /**
     * @brief Read a variable
     * 
     * @param index Index of the current token
     * @param current The current token
     * @param tokens List of all tokens including the current one
     * @param output The output list where the read token will be saved
     * @param globalTokens A list where the tokens that are globally available are saved
     */
    static void ReadVariable(int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens = Language::EmptyTokenList);

    /**
     * @brief Read a field
     * 
     * @param index Index of the current token
     * @param current The current token
     * @param tokens List of all tokens including the current one
     * @param output The output list where the read token will be saved
     * @param globalTokens A list where the tokens that are globally available are saved
     */
    static void ReadField(int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens = Language::EmptyTokenList);

    /**
     * @brief Read an expression
     * 
     * @param index Index of the current token
     * @param current The current token
     * @param tokens List of all tokens including the current one
     * @param output The output list where the read token will be saved
     * @param globalTokens A list where the tokens that are globally available are saved
     * 
     * @returns True if this expression is a return statement
     */
    static bool ReadExpression(int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens = Language::EmptyTokenList);

    /**
     * @brief Read a function
     * 
     * @param index Index of the current token
     * @param current The current token
     * @param tokens List of all tokens including the current one
     * @param output The output list where the read token will be saved
     * @param globalTokens A list where the tokens that are globally available are saved
     */
    static void ReadFunction(int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens = Language::EmptyTokenList);

    /**
     * @brief Read an if-else-statement
     * 
     * @param index Index of the current token
     * @param current The current token
     * @param tokens List of all tokens including the current one
     * @param output The output list where the read token will be saved
     * @param globalTokens A list where the tokens that are globally available are saved
     */
    static void ReadIfStatement(int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens = Language::EmptyTokenList);

    /**
     * @brief Read a loop
     * 
     * @param index Index of the current token
     * @param current The current token
     * @param tokens List of all tokens including the current one
     * @param output The output list where the read token will be saved
     * @param globalTokens A list where the tokens that are globally available are saved
     */
    static void ReadLoop(int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens = Language::EmptyTokenList);

    /**
     * @brief Read a function parameter
     * 
     * @param index Index of the current token
     * @param function Parent function
     * @param current The current token
     * @param tokens List of all tokens including the current one
     */
    static void ReadFunctionParameter(int& index, std::shared_ptr<Language::Function> function, SourceToken& current, SourceTokenList tokens);

    /**
     * @brief Parse tokens and return the value of a variable depending on it's type
     * 
     * @param tokens List of tokens that will be parsed
     * @param varType The type of the variable
     * 
     * @return The value
     */
    static std::string ParseVariableValue(SourceTokenList tokens, Language::VariableType varType);

    /**
     * @brief Read contents (i.e. if/else statements and loops, expressions) that
     *        start with a specific type of token and have a corresponding ending token
     * 
     * @note Will have to be called once it is identified that there are contents,
     *       meaning the next token will NOT be of type `startType`. The current
     *       `index` will be at the last instance of a token with the type `endType`,
     *       meaning to proceed with reading other tokens, first increase the
     *       value of `index`.
     * 
     * 
     * @param index Index of the current token
     * @param startType Token type of the start
     * @param endType Token type of the end
     * @param tokens List of all tokens including the start and end one
     * @param output The output list of all tokens that are considered as content
     * 
     * @returns `true` when the depth is the same as when it was in the beginning,
     *          otherwise `false`
     */
    static bool ReadContentUsingDepth(int& index, SourceToken::Type startType, SourceToken::Type endType, SourceTokenList tokens, SourceTokenList& output);

private:
    static void WriteTokens(BgeFile& outputFile, TokenList& tokenList, std::string indentation = std::string());

private:
    TokenList mPublicTokens;
};

#endif