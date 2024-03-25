#if _WIN32
#   pragma warning(disable: 4996)
#endif

#include "Binaries/CompiledScript.h"
#include "ArgumentParser.h"
#include "TokenParser.h"
#include "StringUtil.h"
#include "Logging.h"
#include "Source.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/stat.h>
#endif

#ifndef _WIN32
#   define DIRECTORYSEPERATOR '/'
#else
#   define DIRECTORYSEPERATOR '\\'
#endif

extern void PrintUsage();
extern void CreatePathDirectory(std::string path);

int main(const int argc, const char** argv)
{
    ////////////////////////
    /// Argument parsing ///
    ////////////////////////

    ArgumentParser parser = ArgumentParser();
    parser.AddArgument(ArgumentType::Flag,  "-h",   "--help");
    parser.AddArgument(ArgumentType::Value, "-o",   "--output");
    parser.AddArgument(ArgumentType::Flag,  "-d",   "--debug-mode");

    parser.ParseArguments(argc - 1, &argv[1]);

    auto fileList = parser.GetUnparsedArguments();
    if (fileList.size() > 1)
    {
        PrintUsage();
        return -1;
    }

    Argument output = parser.GetArgument("-o");

    bool helpRequested = parser.HasArgument("-h");
    bool debugMode = parser.HasArgument("-d");

    //////////////////////
    /// Error handling ///
    //////////////////////

    if (argc < 2 || helpRequested || output == Argument::empty)
    {
        PrintUsage();
        return 0;
    }

    // remove last directory seperator from the output path/file
    if (output.Value.length() > 2)
        if (output.Value.at(output.Value.length()-1) == DIRECTORYSEPERATOR)
            output.Value = output.Value.substr(0, output.Value.length()-1);

    /////////////////////////////////////////
    /// Lexing and parsing of the sources ///
    /////////////////////////////////////////

    Logging::FileLogging = false;
    Logging::Init();

    for (auto& file : fileList)
    {
        Source source;
        BgeFile inputFile = BgeFile(file.Value, false);
        if (!inputFile.Ready())
        {
            Logging::Log(Logging::Format("Error: file '%s' doesn't exist\n", file.Value.c_str()), Logging::Level::Error);
            continue;
        }
        Logging::Log(stringf("Compiling file '%s'", file.Value.c_str()), Logging::Level::Info);
        source.ReadSource(inputFile);
        inputFile.Close();

        if (debugMode)
            source.WriteTokensToJson(std::string(file.Value).append(".tokens.json"));

        TokenParser tokenParser;
        tokenParser.ParseTokens(source.GetTokens());

        if (debugMode)
            tokenParser.WriteTokensToJson(std::string(file.Value).append(".tokens.parsed.json"));

        ///////////////////////////
        /// Compiling to binary ///
        ///////////////////////////

        //////////////////////////
        /// Writing the binary ///
        //////////////////////////

        Binaries::CompiledScript script = Binaries::CompiledScript();
        script.ParseTokens(tokenParser.GetTokens());

        BgeFile outputFile = BgeFile(output.Value, true);
        if (!outputFile.Ready())
        {
            Logging::Log(Logging::Format("Error: cannot create or open file '%s'\n", output.Value.c_str()), Logging::Level::Error);
            continue;
        }
        script.Save(outputFile);
        outputFile.Close();
    }

    Logging::Dispose();

    return 0;
}

void PrintUsage()
{
    printf("bgsc [options] files...\n\n");
    printf("Options:\n");
    printf("\t-i <path>\tSpecify input file\n");
    printf("\t-o <path>\tSpecify output file\n");
    printf("\t-h\t\tShow this help message\n\n");
}

void CreatePathDirectory(std::string path)
{
    std::string pathStr = std::string(path);

    std::vector<std::string> split = StrSplit(pathStr, '/');
    std::string mkdirPath = "";
    for (int i = 0; i < split.size()-1; i++)
    {
        if (i > 0)
            mkdirPath.append("/");

        mkdirPath.append(split.at(i));

        #ifdef _WIN32
            CreateDirectoryA(mkdirPath.c_str(), NULL);
        #else
            mkdir(mkdirPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        #endif
    }
}
