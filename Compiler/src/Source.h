#ifndef __SOURCE_H_
#define __SOURCE_H_ 1

#include "Language/Language.h"
#include <BgeFile.hpp>
#include <string>
#include <vector>

/**
 * @brief A class that contains all information about each line in the source code
 */
struct Source
{
    /**
     * @brief Construct a new `Source` object
     */
    Source();

    /**
     * @brief Construct a new `Source` object
     * 
     * @param file `BgeFile` object that can read the source file
     */
    Source(BgeFile& file);

    /**
     * @brief Append tokens of another source into this source
     * 
     * @param other the other source that will be appened
     */
    void AppendSource(Source& other);

    /**
     * @brief Read and parse a sources contents
     * 
     * @param file `BgeFile` object that can read the source file
     */
    void ReadSource(BgeFile& file);

    /**
     * @brief Get all the analyzed tokens
     * 
     * @return A vector of all tokens
     */
    SourceTokenIterator& GetTokens();

public:
    /**
     * @brief Parse a single line to source tokens
     * 
     * @param str The string that will be parsed
     * 
     * @returns A list of source tokens
     */
    static SourceTokenIterator ParseLineToTokens(std::string str);

private:
    /**
     * @brief Parse a single line and analyze for tokens
     * 
     * @param line Line that will be analyzed
     * @param lineNumber The line number
     */
    void ReadLine(std::string line, int lineNumber);

    /**
     * @brief Analyze/Lex a single buffer that should NOT contain any special characters
     * 
     * @param buffer The buffer that will be analyzed/lexed
     * @param lineNumber The line number
     */
    void ReadBuffer(std::string buffer, int lineNumber);

    /**
     * @brief Read and interpret a single letter
     * 
     * @note This function exists for convenience and for making the code look cleaner
     * 
     * @param chr The character that will be tokenized
     * @param lineNumber The line number
     */
    void ReadLetter(char chr, int lineNumber);

private:
    SourceTokenList mLineTokens;
    SourceTokenIterator mTokens;
};

#endif