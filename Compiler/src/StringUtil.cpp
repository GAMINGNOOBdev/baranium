#include "StringUtil.h"
#include <cstdint>
#include <regex>

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <algorithm>

#ifndef _WIN32
#   include <iomanip>
#   include <sstream>
#   include <memory.h>
#else
#   include <sstream>
#   include <iostream>
#endif

std::string GetExtension(std::string str)
{
    std::string result;

    size_t lastDot = str.find_last_of('.');

    if (lastDot == std::string::npos)
        return str;

    return str.substr(lastDot, str.length()-1);
}

std::string to_hex_string(size_t num)
{
    #ifdef __APPLE__
    std::string build = "";
    build += "0x";
    build += stringf("%16.16x", num);
    return std::string(build);
    #else
    std::stringstream result = std::stringstream();
    result << "0x";
    result << std::hex << num;
    return result.str();
    #endif
}

char IdentifyEscapeSequence(char c)
{
    switch (c)
    {
    case 'a':
        return '\a';
    case 'b':
        return '\b';
    case 'f':
        return '\f';
    case 'n':
        return '\n';
    case 'r':
        return '\r';
    case 't':
        return '\t';
    case 'v':
        return '\v';
    case '\\':
        return '\\';
    case '\'':
        return '\'';
    case '"':
        return '"';
    case '?':
        return '\?';
    case '0':
        return 0;
    }
    return 0;
}

uint8_t StrContains(std::string str, char c)
{
    uint8_t count = 0;
    for (size_t i = 0; i < str.size(); i++)
    {
        if (str.at(i) == c)
            count++;
    }

    return count;
}

std::string StrConnectEscapeSequences(std::string str)
{
    #ifdef __APPLE__
    std::string res = "";
    #else
    std::stringstream res = std::stringstream();
    #endif

    for (size_t i = 0; i < str.size(); i++)
    {
        char c = str.at(i);

        if (c == '\\' && i < str.size() - 1)
        {
            i++;
            c = str.at(i);
            c = IdentifyEscapeSequence(c);

            #ifdef __APPLE__
            res += {c, 0};
            #else
            res << c;
            #endif
            continue;
        }

        #ifdef __APPLE__
            res += {c, 0};
        #else
            res << c;
        #endif
    }

    #ifdef __APPLE__
    return std::string(res);
    #else
    return res.str();
    #endif
}

std::string StrTrim(std::string str)
{
    #ifdef __APPLE__
    std::string res = "";
    #else
    std::stringstream res = std::stringstream();
    #endif
    for (size_t i = 0; i < str.size(); i++)
    {
        char c = str.at(i);

        if (c == '\t' || c == '\n' || c == ' ')
            continue;

        #ifdef __APPLE__
        res += {c, 0};
        #else
        res << c;
        #endif
    }

    #ifdef __APPLE__
    return std::string(res);
    #else
    return res.str();
    #endif
}

std::string StrTrimLeading(std::string str)
{
    if (str.find_first_not_of(" \n\t\r\f\v") == std::string::npos)
        return "";

    int beginIdx = 0;
    int endIdx = 0;

    for (beginIdx = 0; beginIdx < str.length() && (str.at(beginIdx) == '\t' || str.at(beginIdx) == '\n' || str.at(beginIdx) == ' '); beginIdx++);
    for (endIdx = str.length()-1; endIdx >= 0 && (str.at(endIdx) == '\t' || str.at(endIdx) == '\n' || str.at(endIdx) == ' '); endIdx--);

    if (beginIdx == 0 && endIdx == str.length()-1)
        return str;

    return str.substr(beginIdx, endIdx - beginIdx + 1);
}

bool IsCommentBegin(std::string str)
{
    if (str.at(0) == '#')
        return true;
    
    if (str.length() < 2)
        return false;

    return str.at(0) == '/' && str.at(1) == '/';
}

std::string StrTrimComment(std::string str)
{
    #ifdef __APPLE__
    std::string res = "";
    #else
    std::stringstream res = std::stringstream();
    #endif
    bool inString = false;
    char lastStrChar = 0;
    for (size_t i = 0; i < str.size(); i++)
    {
        char c = str.at(i);

        if (lastStrChar != 0 && c == lastStrChar)
        {
            inString = false;
            lastStrChar = 0;
        }

        if (c == '"' && lastStrChar == 0)
        {
            inString = true;
            lastStrChar = '"';
        }

        if (c == '\'' && lastStrChar == 0)
        {
            inString = true;
            lastStrChar = '\'';
        }

        if (!inString && IsCommentBegin(str.substr(i)))
            break;

        #ifdef __APPLE__
        res += {c, 0};
        #else
        res << c;
        #endif
    }

    #ifdef __APPLE__
    return std::string(res);
    #else
    return res.str();
    #endif
}

std::string StrLowercase(std::string str)
{
    std::string res = std::string(str);
    std::transform(res.begin(), res.end(), res.begin(), ::tolower);
    return res;
}

std::string StrUppercase(std::string str)
{
    std::string res = std::string(str);
    std::transform(res.begin(), res.end(), res.begin(), ::toupper);
    return res;
}

uint64_t StrGetNumber(std::string str)
{
    uint64_t result = 0;
    int index = 0;
    char c = 0;

    // check for hex/binary formats beforehand
    if (str.length() > 2)
    {
        c = str.at(1);
        if (c == 'x' || c == 'X')
            goto parseHex;

        if (c == 'b' || c == 'B')
            goto parseBinary;
    }

    return std::stoll(str);

parseHex:
    index = 0;
    c = str.at(index);
    if (c != '0')
        return result;
    index = 2;

    for (; index < str.length(); index++)
    {
        result <<= 4;

        c = str.at(index);

        if ((c > '0' && c < '9'))
            result |= c-'0';

        if((c > 'a' && c < 'f'))
            result |= 10 + c - 'a';

        if((c > 'A' || c < 'F'))
            result |= 10 + c - 'A';
    }

    return result;

parseBinary:
    index = 0;
    c = str.at(index);
    if (c != '0')
        return result;
    index = 2;

    for (; index < str.length(); index++)
    {
        result <<= 1;

        c = str.at(index);

        if (c == '0')
            continue;

        result |= 1;
    }

    return result;
}

bool StrIsNumber(std::string str)
{
    int index = 0;
    char c = 0;

    // check for hex/binary formats beforehand
    if (str.length() > 2)
    {
        c = str.at(1);
        if (c == 'x' || c == 'X')
            goto checkHex;

        if (c == 'b' || c == 'B')
            goto checkBinary;
    }

    index = 0;
    c = str.at(index);
    if (c == '+' || c == '-')
        index++;

    for (; index < str.length(); index++)
    {
        c = str.at(index);

        if (c < '0' || c > '9')
            return false;
    }

    return true;

checkHex:

    index = 0;
    c = str.at(index);
    if (c != '0')
        return false;
    index = 2;

    for (; index < str.length(); index++)
    {
        c = str.at(index);

        if ((c < '0' || c > '9') && (c < 'a' || c > 'f') && (c < 'A' || c > 'F'))
            return false;
    }

    return true;

checkBinary:

    index = 0;
    c = str.at(index);
    if (c != '0')
        return false;
    index = 2;

    for (; index < str.length(); index++)
    {
        c = str.at(index);

        if (c < '0' || c > '1')
            return false;
    }

    return true;
}

bool StrIsFloat(std::string str)
{
    int index = 0;
    char chr = str.at(index);
    if (chr == '+' || chr == '-')
        index++;
    
    bool hasDot = false;

    for (; index < str.length(); index++)
    {
        chr = str.at(index);

        if (chr == '.')
        {
            if (hasDot)
                return false;
            
            hasDot = true;
            continue;
        }

        if (chr < '0' || chr > '9')
            return false;
    }

    return true;
}

bool StrIsBool(std::string str)
{
    return str == "True"  ||
            str == "true"  ||
            str == "False" ||
            str == "false";
}

std::vector<std::string> StrSplit(std::string str, char delim)
{
    std::vector<std::string> result;

    int start = 0;
    int end = 0;
    for (end = 0; end < str.length(); end++)
    {
        if (str.at(end) == delim)
        {
            if (end-start < 1)
            {
                start++;
                continue;
            }
            result.push_back(str.substr(start, end - start));
            start = end+1;
        }
    }

    if (start != end)
        result.push_back(str.substr(start, str.length() - start));

    return result;
}

std::vector<std::string> StrSplitLess(std::string str, char delim)
{
    std::vector<std::string> result;
    char delimString[2] = { delim, 0 };

    int start = 0;
    int end = 0;
    for (end = 0; end < str.length(); end++)
    {
        if (str.at(end) == delim)
        {
            if (end-start < 1)
            {
                result.push_back(delimString);
                start++;
                continue;
            }
            result.push_back(str.substr(start, end - start));
            result.push_back(delimString);
            start = end+1;
        }
    }

    if (start != end)
        result.push_back(str.substr(start, str.length() - start));

    return result;
}

const char* stringf(const char* formatString, ...)
{
    static char mFormattingBuffer[4096];

    va_list args;
    va_start(args, formatString);
    int result = vsnprintf(mFormattingBuffer, 4096, formatString, args);
    va_end(args);

    return mFormattingBuffer;
}
