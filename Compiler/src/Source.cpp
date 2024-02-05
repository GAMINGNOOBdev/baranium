#include "StringUtil.h"
#include <algorithm>
#include "Source.h"

/**
 * @brief Checks if the given string is a keyword
 * 
 * @param string The string that will be checked if it is a keyword
 * 
 * @returns -1 if not a keyword, else it will return the index of it
 */
int IsKeyword(std::string string)
{
    auto iterator = std::find_if(Language::Keywords.begin(), Language::Keywords.end(),
        [string](Language::Keyword& a)
        {
            return a.Name == string;
        }
    );
    if (iterator == Language::Keywords.end())
        return -1;

    return iterator - Language::Keywords.begin();
}

/**
 * @brief Construct a new `Source` object
 */
Source::Source()
{
}

/**
 * @brief Construct a new `Source` object
 * 
 * @param file `BgeFile` object that can read the source file
 */
Source::Source(BgeFile& file)
{
    ReadSource(file);
}

/**
 * @brief Write the source code tokens into a single json file
 * 
 * @note Executed then the `-d` or `--debug` flag is passed to the main executable
 * 
 * @param name Filename of the json file where the token data will be written to
 */
void Source::WriteTokensToJson(std::string name)
{
    BgeFile outputFile = BgeFile(name, true);
    outputFile.WriteLine("[");
    int index = 0;
    int tokenCount = mTokens.size();
    for (auto& token : mTokens)
    {
        outputFile.WriteLine("\t{");
        outputFile.WriteLine(stringf("\t\t\"type\": \"%s\",", SourceTokenTypeToString(token.mType)));
        outputFile.WriteLine(stringf("\t\t\"line\": \"%d\",", token.LineNumber));
        if (token.mType == SourceToken::Type::DoubleQuote)
            outputFile.WriteLine(stringf("\t\t\"contents\": \"\\\"\""));
        else
            outputFile.WriteLine(stringf("\t\t\"contents\": \"%s\"", token.Contents.c_str()));

        if (index == tokenCount-1)
            outputFile.WriteLine("\t}");
        else
            outputFile.WriteLine("\t},");
        index++;
    }
    outputFile.WriteLine("]\n");
    outputFile.Close();
}

/**
 * @brief Read and parse a sources contents
 * 
 * @param file `BgeFile` object that can read the source file
 */
void Source::ReadSource(BgeFile& file)
{
    if (!file.Ready())
        return;

    std::string currentLine = "";
    std::string line = "";

    int lineNumber = 0;
    while (!(currentLine = file.ReadLine()).empty() || !file.EndOfFile())
    {
        lineNumber++;
        if (currentLine.empty())
            continue;

        line = StrTrimLeading(currentLine);
        if (line.empty() || line[0] == '#' || line.substr(0, 2) == "//")
            continue;

        line = StrTrimLeading( StrTrimComment(line) );

        if (line[0] == '+')
        {
            // TODO: preprocessor stuff
            continue;
        }

        ReadLine(line, lineNumber);
    }
}

/**
 * @brief Get all the analyzed tokens
 * 
 * @return A vector of all tokens
 */
std::vector<SourceToken>& Source::GetTokens()
{
    return mTokens;
}

/**
 * @brief Parse a single line and analyze for tokens
 * 
 * @param line Line that will be analyzed
 * @param lineNumber The line number
 */
void Source::ReadLine(std::string line, int lineNumber)
{
    if (line.empty())
        return;

    if (line.length() < 2)
    {
        ReadLetter(line.at(0), lineNumber);
        return;
    }

    int start = 0;
    int end = 0;
    char tmpStr[2] = {0, 0};
    bool inString = false;
    char lastStringChar = 0;

    for (int index = 0; index < line.length(); index++)
    {
        tmpStr[0] = line.at(index);
        char chr = tmpStr[0];
        if (isspace(chr) && !inString)
        {
            end++;
            ReadBuffer(line.substr(start, end - start), lineNumber);
            start = index+1;
            continue;
        }

        if (chr == lastStringChar && inString)
            inString = false;

        if (!inString)
        for (auto& specialCharacter : Language::SpecialCharacters)
        {
            if (chr == specialCharacter.Character)
            {
                if (chr == '"' && lastStringChar == 0)
                {
                    inString = true;
                    lastStringChar = chr;
                }

                if (!inString && chr == '"' && lastStringChar != 0)
                    lastStringChar = 0;

                end++;
                ReadBuffer(line.substr(start, end - start), lineNumber);
                start = index+1;

                SourceToken token = SourceToken();
                token.Contents = std::string(tmpStr);
                token.mType = specialCharacter.TokenType;
                token.KeywordIndex = -1;
                token.LineNumber = lineNumber;
                mTokens.push_back(token);

                break;
            }
        }

        end = index;
    }

    if (start == 0 && end == line.size()-1)
        ReadBuffer(line, lineNumber);
}

/**
 * @brief Analyze/Lex a single buffer that should NOT contain any special characters
 * 
 * @param buffer The buffer that will be analyzed/lexed
 * @param lineNumber The line number
 */
void Source::ReadBuffer(std::string buffer, int lineNumber)
{
    if (buffer.empty())
        return;

    if (StrIsNumber(buffer))
    {
        SourceToken numberToken = SourceToken();
        numberToken.KeywordIndex = -1;
        numberToken.Contents = buffer;
        numberToken.LineNumber = lineNumber;
        numberToken.mType = SourceToken::Type::Number;
        mTokens.push_back(numberToken);
        return;
    }

    SourceToken token = SourceToken();

    int keywordIndex = IsKeyword(buffer);
    token.KeywordIndex = keywordIndex;
    token.LineNumber = lineNumber;
    token.mType = SourceToken::Type::Text;
    
    if (keywordIndex != -1)
    {
        auto& keyword = Language::Keywords[keywordIndex];
        token.Contents = std::string(keyword.Name);
        token.mType = keyword.TokenType;
        mTokens.push_back(token);
        return;
    }

    token.Contents = std::string(buffer);
    mTokens.push_back(token);
}

/**
 * @brief Read and interpret a single letter
 * 
 * @note This function exists for convenience and for making the code look cleaner
 * 
 * @param chr The character that will be tokenized
 * @param lineNumber The line number
 */
void Source::ReadLetter(char chr, int lineNumber)
{
    SourceToken token;
    token.KeywordIndex = -1;
    token.LineNumber = lineNumber;
    token.mType = SourceToken::Type::Text;
    char letterString[2] = {chr, 0};
    token.Contents = std::string(letterString);

    if (isdigit(chr))
    {
        token.mType = SourceToken::Type::Number;
        goto end;
    }

    for (auto& specialCharacter : Language::SpecialCharacters)
    {
        if (chr == specialCharacter.Character)
        {
            token.mType = specialCharacter.TokenType;
            break;
        }
    }

end:

    mTokens.push_back(token);
}