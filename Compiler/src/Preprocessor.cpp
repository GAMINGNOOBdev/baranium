#include "Preprocessor.h"
#include "StringUtil.h"
#include "FileUtil.h"
#include "Logging.h"
#include <algorithm>

#ifdef _WIN32
#   undef min
#   undef max
#endif

std::vector<std::string> Preprocessor::mIncludePaths;

void Preprocessor::Parse(std::string operation, Source* source)
{
    if (operation.empty() || source == nullptr)
        return;
    
    // kinda a bruteforce method but i don't care, there probably isn't gonna be anything BUT including anyways
    // well, probably for now until literally anyone cares and sees this project and people actually use this
    // stuff

    if (operation.length() < 7)
        return;
    
    if (StrTrimLeading(operation) == "include")
    {
        printf("haha, funni. including nothing, huh? well that's certainly strange but i know something stranger and it's the fact that there even is a message like this that catches that specific case of not having a single file that you want to include in a include statement. you really deserve a gold medal for finding this secret little easter egg my friend. have a good day.");
        return;
    }

    // make sure this is actually an include
    if (operation.substr(0,7) != "include")
        return;

    std::string pathsString = operation.substr(7);
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
        src.GetTokens().clear();
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