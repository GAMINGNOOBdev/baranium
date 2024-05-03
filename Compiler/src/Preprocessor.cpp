#include "Language/Language.h"
#include "Preprocessor.h"
#include <unordered_map>
#include "StringUtil.h"
#include "FileUtil.h"
#include "Logging.h"
#include <algorithm>

#ifdef _WIN32
#   undef min
#   undef max
#endif

std::vector<std::string> Preprocessor::mIncludePaths = std::vector<std::string>();
std::vector<std::string> Preprocessor::mDefinesEntries = std::vector<std::string>();
std::vector<std::string> Preprocessor::mDefines = std::vector<std::string>();

void Preprocessor::Parse(std::string operation, Source* source)
{
    if (operation.empty() || source == nullptr)
        return;

    std::string cleanBuffer = StrTrimLeading(operation);

    if (cleanBuffer == "include")
    {
        printf("haha, funni. including nothing, huh? well that's certainly strange but i know something stranger and it's the fact that there even is a message like this that catches that specific case of not having a single file that you want to include in a include statement. you really deserve a gold medal for finding this secret little easter egg my friend. have a good day.\n");
        return;
    }

    std::string command = cleanBuffer.substr(0, cleanBuffer.find_first_of(' '));
    // there are currently no preprocessor commands that are just standalone
    if (command.empty())
        return;

    // make sure this is actually an include
    if (command == "include")
    {
        std::string pathsString = cleanBuffer.substr(7);
        pathsString = StrTrimLeading(pathsString);
        auto paths = StrSplit(pathsString, ',');
        std::string includeFile;
        std::string includePath;
        for (auto& not_safe_for_work_path : paths)
        {
            includeFile = StrTrimLeading(not_safe_for_work_path);
            includePath = SearchIncludePath(includeFile);
            if (includePath.empty())
                Logging::LogErrorExit(stringf("Including file '%s' failed", includeFile.c_str()), -includeFile.length());

            BgeFile file = BgeFile(includePath, false);

            if (!file.Ready())
                Logging::LogErrorExit(stringf("Including file '%s' failed", includeFile.c_str()), -includeFile.length());

            Source src = Source();
            src.ReadSource(file);
            source->AppendSource(src);
            file.Close();
            src.GetTokens().Clear();
        }
    }
    else if (command == "define")
    {
        std::string nameAndReplacement = StrTrimLeading(cleanBuffer.substr(6));
        if (nameAndReplacement.empty())
            return;

        size_t nameSplitIndex = nameAndReplacement.find_first_of(' ');
        if (nameSplitIndex == std::string::npos)
        {
            AddDefine(nameAndReplacement, "");
            return;
        }

        AddDefine(StrTrimLeading(nameAndReplacement.substr(0,nameSplitIndex)), StrTrimLeading(nameAndReplacement.substr(nameSplitIndex)));
    }
}

void Preprocessor::AddIncludePath(std::string path)
{
    std::string includePath = path;
    if (includePath.at(includePath.length()-1) == '\\' || includePath.at(includePath.length()-1) == '/')
        includePath = includePath.substr(0, includePath.length()-1);

    if (std::find(mIncludePaths.begin(), mIncludePaths.end(), includePath) != mIncludePaths.end())
        return;

    mIncludePaths.push_back(includePath);
}

void Preprocessor::PopLastInclude()
{
    if (mIncludePaths.size() == 1)
        mIncludePaths.clear();

    if (mIncludePaths.size() > 2)
        mIncludePaths.erase(mIncludePaths.end());
}

void Preprocessor::AddDefine(std::string defineName, std::string replacement)
{
    if (defineName.empty())
        return;

    if (std::find(mDefinesEntries.begin(), mDefinesEntries.end(), defineName) != mDefinesEntries.end())
        return;

    if (Language::IsKeyword(defineName) != -1 || StrIsNumber(defineName) ||
        isdigit(defineName[0]) || Language::IsSpecialChar(defineName[0]) != -1)
        return;

    Logging::Log(stringf("define{'%s'} replacement{'%s'}", defineName.c_str(), replacement.c_str()));

    mDefinesEntries.push_back(defineName);
    mDefines.push_back(replacement);
}

void Preprocessor::AssistInLine(std::vector<SourceToken>& lineTokens)
{
    std::vector<SourceToken> improvedTokens;

    if (lineTokens.empty())
        return;
    
    bool anythingChanged = false;
    for (auto iterator = lineTokens.begin(); iterator != lineTokens.end(); iterator++)
    {
        auto& token = *iterator;
        bool defineFound = false;
        int index = 0;
        for (auto& define : mDefinesEntries)
        {
            if (token.Contents != define)
            {
                index++;
                break;
            }
            defineFound = true;
            anythingChanged = true;

            if (mDefines[index].empty())
                break;

            auto replacementTokensIterator = Source::ParseLineToTokens(mDefines[index]);
            auto replacementTokens = replacementTokensIterator.GetTokens();
            if (replacementTokens.empty())
                break;

            if (replacementTokens.size() == 1)
            {
                replacementTokens[0].LineNumber = token.LineNumber;
                improvedTokens.push_back(replacementTokens[0]);
                break;
            }

            for (auto& replacement : replacementTokens)
                replacement.LineNumber = token.LineNumber;

            improvedTokens.insert(improvedTokens.end(), replacementTokens.begin(), replacementTokens.end());
            break;
        }

        if (!defineFound)
            improvedTokens.push_back(token);
    }
    if (!anythingChanged)
        return;

    lineTokens.clear();
    lineTokens.insert(lineTokens.end(), improvedTokens.begin(), improvedTokens.end());
}

std::string Preprocessor::SearchIncludePath(std::string file)
{
    for (auto& includePath : mIncludePaths)
    {
        for (auto& fileName : FileUtil::GetDirectoryContents(includePath, FileUtil::FilterMask::Files))
        {
            if (fileName == file)
                return stringf("%s/%s", includePath.c_str(), file.c_str());
        }
    }

    return "";
}