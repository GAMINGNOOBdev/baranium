#include "Preprocessor.h"
#include "StringUtil.h"
#include "Logging.h"

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
    std::string path;
    for (auto& not_safe_for_work_path : paths)
    {
        path = StrTrimLeading(not_safe_for_work_path);
        BgeFile file = BgeFile(path, false);
        if (!file.Ready())
            Logging::LogErrorExit(stringf("Including file '%s' failed", path.c_str()), -path.length());

        Source src = Source();
        src.ReadSource(file);
        source->AppendSource(src);
        file.Close();
        src.GetTokens().clear();
    }
}