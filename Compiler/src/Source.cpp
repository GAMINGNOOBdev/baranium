#include "Preprocessor.h"
#include "StringUtil.h"
#include <algorithm>
#include "Source.h"

Source::Source()
{
}

Source::Source(BgeFile& file)
{
    ReadSource(file);
}

void Source::AppendSource(Source& other)
{
    auto& otherTokens = other.GetTokens();
    mTokens.Push(otherTokens);
}

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
            Preprocessor::Parse(line.substr(1), this);
            continue;
        }

        ReadLine(line, lineNumber);
    }
}

SourceTokenIterator& Source::GetTokens()
{
    return mTokens;
}

SourceTokenIterator Source::ParseLineToTokens(std::string line)
{
    Source src = Source();
    src.ReadLine(line, -1);
    return SourceTokenIterator(src.GetTokens());
}

///////////////////////
/// Private methods ///
///////////////////////

void Source::ReadLine(std::string line, int lineNumber)
{
    int start = 0;
    int end = 0;
    char tmpStr[2] = {0, 0};
    bool inString = false;
    char lastStringChar = 0;
    char chr = tmpStr[0];

    if (line.empty())
        return;

    if (line.length() < 2)
    {
        ReadLetter(line.at(0), lineNumber);
        goto validate;
    }

    for (int index = 0; index < line.length(); index++)
    {
        tmpStr[0] = line.at(index);
        chr = tmpStr[0];
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
        {
            int specialCharIndex = Language::IsSpecialChar(chr);
            if (specialCharIndex != -1)
            {
                if (chr == '"' && lastStringChar == 0)
                {
                    inString = true;
                    lastStringChar = chr;
                }

                if (!inString && chr == '"' && lastStringChar != 0)
                    lastStringChar = 0;

                end++;
                if (line.substr(start, end - start) != tmpStr)
                    ReadBuffer(line.substr(start, end - start), lineNumber);
                start = index+1;

                int specialOperatorIndex = -1;
                if (mLineTokens.size() > 0)
                {
                    auto lastToken = mLineTokens.back();
                    specialOperatorIndex = Language::IsSpecialOperator(lastToken.Contents.at(0), chr);
                    if (Language::IsSpecialChar(lastToken.Contents.at(0)) != -1 && specialOperatorIndex != -1)
                    {
                        mLineTokens.pop_back();
                        SourceToken token = SourceToken();
                        token.Contents = Language::SpecialOperators[specialOperatorIndex].Operator;
                        token.mType = Language::SpecialOperators[specialOperatorIndex].TokenType;
                        token.KeywordIndex = -1;
                        token.LineNumber = lineNumber;
                        mLineTokens.push_back(token);
                    }
                }
                if (specialOperatorIndex == -1)
                {
                    SourceToken token = SourceToken();
                    token.Contents = std::string(tmpStr);
                    token.mType = Language::SpecialCharacters[specialCharIndex].TokenType;
                    token.KeywordIndex = -1;
                    token.LineNumber = lineNumber;
                    mLineTokens.push_back(token);
                }
            }
        }

        end = index;
    }

    if (start == 0 && end == line.size()-1)
        ReadBuffer(line, lineNumber);

validate:
    Preprocessor::AssistInLine(mLineTokens);
    mTokens.Push(mLineTokens);
    mLineTokens.clear();
}

void Source::ReadBuffer(std::string buffer, int lineNumber)
{
    buffer = StrTrimLeading(buffer);

    if (buffer.empty())
        return;

    if (StrIsNumber(buffer))
    {
        SourceToken numberToken = SourceToken();
        numberToken.KeywordIndex = -1;
        numberToken.Contents = buffer;
        numberToken.LineNumber = lineNumber;
        numberToken.mType = SourceToken::Type::Number;
        mLineTokens.push_back(numberToken);
        return;
    }

    SourceToken token = SourceToken();

    int keywordIndex = Language::IsKeyword(buffer);
    token.KeywordIndex = keywordIndex;
    token.LineNumber = lineNumber;
    token.mType = SourceToken::Type::Text;
    token.Contents = std::string(buffer);

    if (keywordIndex != -1)
    {
        auto& keyword = Language::Keywords[keywordIndex];
        token.Contents = std::string(keyword.Name);
        token.mType = keyword.TokenType;
    }

    mLineTokens.push_back(token);
}

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

    mLineTokens.push_back(token);
}