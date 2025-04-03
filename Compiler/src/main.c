#if _WIN32
#   pragma warning(disable: 4996)
#endif

#include <baranium/compiler/language/abstract_syntax_tree.h>
#include <baranium/compiler/language/expression_token.h>
#include <baranium/compiler/language/function_token.h>
#include <baranium/compiler/language/if_else_token.h>
#include <baranium/compiler/binaries/compiler.h>
#include <baranium/compiler/language/token.h>
#include <baranium/compiler/preprocessor.h>
#include <baranium/compiler/token_parser.h>
#include <baranium/compiler/source.h>
#include <baranium/string_util.h>
#include <baranium/file_util.h>
#include <baranium/logging.h>
#include <argument_parser.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#   include <Windows.h>
#   define OS_DELIMITER '\\'
#   undef max
#   undef min
#else
#   include <sys/stat.h>
#   include <unistd.h>
#   define OS_DELIMITER '/'
#endif

#define PRINT_VERSION printf("Baranium runtime version %d.%d.%d %s\n", BARANIUM_VERSION_YEAR, BARANIUM_VERSION_MONTH, BARANIUM_VERSION_DATE, BARANIUM_VERSION_PHASE)

void print_usage(void);

uint8_t g_debug_mode = 0;

const char* strptr(const char* src)
{
    size_t len = strlen(src)+1;
    char* result = malloc(len);
    memcpy(result,src,len-1);
    result[len-1]=0;
    return (const char*)result;
}

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
size_t getdelim(char **buffer, size_t *buffersz, FILE *stream, char delim) {
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (buffer == NULL) {
        return -1;
    }
    if (stream == NULL) {
        return -1;
    }
    if (buffersz == NULL) {
        return -1;
    }
    bufptr = *buffer;
    size = *buffersz;

    c = fgetc(stream);
    if (c == EOF) {
        return -1;
    }
    if (bufptr == NULL) {
        bufptr = malloc(128);
        if (bufptr == NULL) {
            return -1;
        }
        size = 128;
    }
    p = bufptr;
    while(c != EOF) {
        if ((p - bufptr) > (size - 1)) {
            size = size + 128;
            bufptr = realloc(bufptr, size);
            if (bufptr == NULL) {
                return -1;
            }
        }
        *p++ = c;
        if (c == delim) {
            break;
        }
        c = fgetc(stream);
    }

    *p++ = '\0';
    *buffer = bufptr;
    *buffersz = size;

    return p - bufptr - 1;
}

size_t getline(char **buffer, size_t *buffersz, FILE *stream)
{
    return getdelim(buffer, buffersz, stream, '\n');
}
#endif

void read_includes_file(const char* path)
{
    FILE* includePathsFile = fopen(path, "r");
    if (includePathsFile == NULL)
    {
        LOGERROR(stringf("Can't open includes file '%s': No such file or directory", path));
        free((void*)path);
        return;
    }

    char* line = NULL;
    size_t linesize = 0;
    while (getline(&line, &linesize, includePathsFile) != EOF)
    {
        line[linesize-1] = 0;
        linesize--;
        baranium_preprocessor_add_include_path(line);
        if (line != NULL)
            free(line);
    }
    fclose(includePathsFile);
    free((void*)path);
}

char* get_executable_working_directory(void)
{
    char* result = (char*)malloc(0x1000);

    #ifdef _WIN32
        DWORD status = GetModuleFileNameA(NULL, &result[0], 0x1000);
        if (status == ERROR)
            return "";
    #else
        if (readlink("/proc/self/exe", result, 0x1000) == -1)
            return "";
    #endif

    return result;
}

size_t str_index_of(const char* string, char delim)
{
    size_t stringLastSeperatorIndex = -1;
    for (size_t i = 0; string[i] != 0; i++)
        stringLastSeperatorIndex = (string[i] == delim) ? i : stringLastSeperatorIndex;

    return stringLastSeperatorIndex;
}

int main(const int argc, const char** argv)
{
    ////////////////////////
    /// Argument parsing ///
    ////////////////////////

    if (argc < 2)
    {
        print_usage();
        return 0;
    }

    ArgumentParser* parser = argument_parser_init();
    argument_parser_add(parser, ArgumentType_Flag, "-h", "--help");
    argument_parser_add(parser, ArgumentType_Flag, "-d", "--debug");
    argument_parser_add(parser, ArgumentType_Flag, "-v", "--version");
    argument_parser_add(parser, ArgumentType_Value, "-o", "--output");
    argument_parser_add(parser, ArgumentType_Value, "-I", "--include");
    argument_parser_parse(parser, argc, argv);

    if (argument_parser_has(parser, "-h"))
    {
        print_usage();
        argument_parser_dispose(parser);
        return 0;
    }

    if (argument_parser_has(parser, "-v"))
    {
        PRINT_VERSION;
        argument_parser_dispose(parser);
        return 0;
    }

    g_debug_mode = argument_parser_has(parser, "-d");
    logEnableDebugMsgs(g_debug_mode);
    logEnableStdout(1); // should always be on
    FILE* logOutput = fopen("runtime.log", "wb+");
    logSetStream(logOutput);

    char* output = "output.bin";
    uint8_t outputPathPresent = argument_parser_has(parser, "-o");
    Argument* userIncludes = argument_parser_get(parser, "-I");

    baranium_preprocessor_init();

    // read include paths
    char* executableFilePath = get_executable_working_directory();
    size_t executableFilePathLastSeperatorIndex = str_index_of(executableFilePath, OS_DELIMITER);
    if (executableFilePathLastSeperatorIndex != (size_t)-1)
        executableFilePath[executableFilePathLastSeperatorIndex] = 0;

    char* baraniumInclude = getenv("BARANIUM_INCLUDE");
    if (baraniumInclude != NULL)
        baranium_preprocessor_add_include_path(baraniumInclude);
    baranium_preprocessor_add_include_path(stringf("%s%s", executableFilePath, "../include"));
    baranium_preprocessor_add_include_path(stringf("%s%s", executableFilePath, "include"));
    baranium_preprocessor_add_include_path(executableFilePath);

    char* baraniumIncludeFile = getenv("BARANIUM_INCLUDE_CONFIG");
    if (baraniumIncludeFile != NULL)
        read_includes_file(baraniumIncludeFile);
    read_includes_file(strsubstr(stringf("%s/%s", executableFilePath, "../etc/include.paths"),0,-1));
    read_includes_file(strsubstr(stringf("%s/%s", executableFilePath, "../include.paths"),0,-1));
    read_includes_file(strsubstr(stringf("%s/%s", executableFilePath, "etc/include.paths"),0,-1));
    read_includes_file(strsubstr(stringf("%s/%s", executableFilePath, "include.paths"),0,-1));

    if (userIncludes != NULL)
        read_includes_file(userIncludes->Value);


    //////////////////////
    /// Error handling ///
    //////////////////////

    if (outputPathPresent)
    {
        Argument* outputArg = argument_parser_get(parser, "-o");
        output = (char*)outputArg->Value;
    }

    // remove last directory seperator from the output path/file
    if (strlen(output) > 2)
    {
        size_t outputLastSeperatorIndex = str_index_of(output, OS_DELIMITER);

        if (outputLastSeperatorIndex == strlen(output)-1)
            output[outputLastSeperatorIndex] = 0;
    }

    /////////////////////////////////////////
    /// Lexing and parsing of the sources ///
    /////////////////////////////////////////

    baranium_source_token_list combinedSource;
    baranium_source_token_list_init(&combinedSource);
    for (Argument* file = parser->unparsed->start; file != NULL; file = file->next)
    {
        char* sourceFileDirectory = (char*)file->Value;
        size_t sourceFileDirectorySeperatorIndex = str_index_of(sourceFileDirectory, OS_DELIMITER);
        if (sourceFileDirectorySeperatorIndex == (size_t)-1)
            sourceFileDirectory = ".";
        else
            sourceFileDirectory[sourceFileDirectorySeperatorIndex] = 0;
        baranium_preprocessor_add_include_path(sourceFileDirectory);
        if (sourceFileDirectorySeperatorIndex != (size_t)-1)
            sourceFileDirectory[sourceFileDirectorySeperatorIndex] = OS_DELIMITER;

        baranium_source_token_list source;
        baranium_source_token_list_init(&source);
        FILE* inputFile = fopen(file->Value, "r");
        if (inputFile == NULL)
        {
            LOGERROR(stringf("Error: file '%s' doesn't exist\n", file->Value));
            continue;
        }
        LOGINFO(stringf("Compiling file '%s'", file->Value));
        baranium_source_open_from_file(&source, inputFile);
        fclose(inputFile);

        baranium_source_token_list_push_list(&combinedSource, &source);
        baranium_source_token_list_dispose(&source, 1);

        baranium_preprocessor_pop_last_include();
    }

    baranium_preprocessor_dispose();

    baranium_token_parser token_parser;
    baranium_token_parser_init(&token_parser);
    baranium_token_parser_parse(&token_parser, &combinedSource);

    ////////////////////////////////////////
    /// Compiling and writing the binary ///
    ////////////////////////////////////////

    FILE* outputFile = fopen(output, "wb+");
    if (outputFile == NULL)
    {
        LOGERROR(stringf("Error: cannot create or open file '%s'\n", output));
        goto end;
    }

    baranium_compiler compiler;
    baranium_compiler_init(&compiler);
    baranium_compiler_write(&compiler, &token_parser.tokens, outputFile);
    baranium_compiler_dispose(&compiler);

    fclose(outputFile);

end:
    baranium_token_parser_dispose(&token_parser);
    baranium_source_token_list_dispose(&combinedSource, 1);

    free(executableFilePath);
    argument_parser_dispose(parser);
    fclose(logOutput);

    return 0;
}

void print_usage(void)
{
    printf("barc [options] files...\n\n");
    printf("Options:\n");
    printf("\t-o <path>\tSpecify output file\n");
    printf("\t-h\t\tShow this help message\n");
    printf("\t-I <file>\tSpecify file containing all custom user include directories\n");
    printf("\t-d\t\tPrint debug messages (only useful for debugging the compiler itself!)\n\n");
}
