#if _WIN32
#   pragma warning(disable: 4996)
#endif

#include "Binaries/CompiledScript.h"
#include "ArgumentParser.h"
#include "Preprocessor.h"
#include "TokenParser.h"
#include "StringUtil.h"
#include "FileUtil.h"
#include "Logging.h"
#include "Source.h"

#ifdef _WIN32
#   include <Windows.h>
#   define OS_DELIMITER '\\'
#   undef max
#   undef min
#else
#   include <sys/stat.h>
#   define OS_DELIMITER '/'
#endif

extern void PrintUsage();
extern void CreatePathDirectory(std::string path);

bool g_debug_mode = false;

int main(const int argc, const char** argv)
{
    ////////////////////////
    /// Argument parsing ///
    ////////////////////////

    ArgumentParser parser = ArgumentParser();
    parser.AddArgument(ArgumentType::Flag,  "-h",   "--help");
    parser.AddArgument(ArgumentType::Value, "-o",   "--output");
    parser.AddArgument(ArgumentType::Flag,  "-d",   "--debug-mode");
    parser.AddArgument(ArgumentType::Value, "-I", "--include");

    parser.ParseArguments(argc - 1, &argv[1]);

    auto fileList = parser.GetUnparsedArguments();
    Argument output = parser.GetArgument("-o");
    Argument userIncludes = parser.GetArgument("-I");

    bool helpRequested = parser.HasArgument("-h");
    g_debug_mode = parser.HasArgument("-d");
    Logging::DebugMessages = g_debug_mode;

    // read include paths
    std::string executableFilePath = FileUtil::GetExecutableWorkingDirectory();
    size_t executableFilePathLastSeperatorIndex = executableFilePath.find_last_of(OS_DELIMITER);
    if (executableFilePathLastSeperatorIndex != std::string::npos)
        executableFilePath = executableFilePath.substr(0, executableFilePathLastSeperatorIndex+1); // index + 1 to include the delimiter

    Preprocessor::AddIncludePath(std::string(executableFilePath).append("../include"));

    std::string includePathsFilePath = std::string(executableFilePath).append("../etc/include.paths");
    if (FileUtil::Exists(includePathsFilePath))
    {
        BgeFile includePathsFile = BgeFile(includePathsFilePath, false);
        if (!includePathsFile.Ready())
            Logging::LogErrorExit("how in the f*ck did this error occur?!?!?!?");

        std::string line;
        while (!includePathsFile.EndOfFile())
        {
            line = includePathsFile.ReadLine();
            if (line.empty())
                continue;

            Preprocessor::AddIncludePath(line);
        }
        includePathsFile.Close();
    }

    if (userIncludes != Argument::empty)
    {
        if (FileUtil::Exists(userIncludes.Value))
        {
            BgeFile userIncludePathsFile = BgeFile(userIncludes.Value, false);

            if (!userIncludePathsFile.Ready())
                Logging::LogErrorExit("again, how the f*ck did you get here?");

            std::string line;
            while (!userIncludePathsFile.EndOfFile())
            {
                line = userIncludePathsFile.ReadLine();
                if (line.empty())
                    continue;

                Preprocessor::AddIncludePath(line);
            }
            userIncludePathsFile.Close();
        }
    }

    //////////////////////
    /// Error handling ///
    //////////////////////

    if (output == Argument::empty)
    {
        output = Argument();
        output.Value = "output.bin";
    }

    if (argc < 2 || helpRequested)
    {
        PrintUsage();
        return 0;
    }

    // remove last directory seperator from the output path/file
    if (output.Value.length() > 2)
        if (output.Value.at(output.Value.length()-1) == '\\' || output.Value.at(output.Value.length()-1) == '/')
            output.Value = output.Value.substr(0, output.Value.length()-1);

    /////////////////////////////////////////
    /// Lexing and parsing of the sources ///
    /////////////////////////////////////////

    Logging::FileLogging = false;
    Logging::Init();

    Source combinedSource;
    for (auto& file : fileList)
    {
        std::string sourceFileDirectory = file.Value;
        size_t sourceFileDirectorySeperatorIndex = std::max((int64_t)sourceFileDirectory.find_last_of('\\'), (int64_t)sourceFileDirectory.find_last_of('/'));
        if (sourceFileDirectorySeperatorIndex == std::string::npos)
            sourceFileDirectory = "";
        else
        {
            sourceFileDirectory = sourceFileDirectory.substr(0, sourceFileDirectorySeperatorIndex);
            Preprocessor::AddIncludePath(sourceFileDirectory);
        }

        Source source;
        BgeFile inputFile = BgeFile(file.Value, false);
        if (!inputFile.Ready())
        {
            Logging::Log(stringf("Error: file '%s' doesn't exist\n", file.Value.c_str()), Logging::Level::Error);
            continue;
        }
        Logging::Log(stringf("Compiling file '%s'", file.Value.c_str()), Logging::Level::Info);
        source.ReadSource(inputFile);
        inputFile.Close();

        combinedSource.AppendSource(source);

        Preprocessor::PopLastInclude();
    }

    TokenParser tokenParser;
    tokenParser.ParseTokens(combinedSource.GetTokens());

    ////////////////////////////////////////
    /// Compiling and writing the binary ///
    ////////////////////////////////////////

    Binaries::CompiledScript script = Binaries::CompiledScript();
    script.ParseTokens(tokenParser.GetTokens());

    BgeFile outputFile = BgeFile(output.Value, true);
    if (!outputFile.Ready())
        Logging::LogErrorExit(stringf("Error: cannot create or open file '%s'\n", output.Value.c_str()));

    script.Save(outputFile);
    outputFile.Close();

    Logging::Dispose();

    return 0;
}

void PrintUsage()
{
    printf("barc [options] files...\n\n");
    printf("Options:\n");
    printf("\t-o <path>\tSpecify output file\n");
    printf("\t-h\t\tShow this help message\n");
    printf("\t-I <file>\tSpecify file containing all custom user include directories\n");
    printf("\t-d\t\tPrint debug messages (only useful for debugging the compiler itself!)\n\n");
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
