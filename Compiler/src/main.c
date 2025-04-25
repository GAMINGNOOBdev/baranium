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
#include <baranium/compiler/compiler_context.h>
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

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
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
size_t getdelimv2(char **buffer, size_t *buffersz, FILE *stream, char delim) {
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

size_t getlinev2(char **buffer, size_t *buffersz, FILE *stream)
{
    return getdelimv2(buffer, buffersz, stream, '\n');
}
#endif

void read_includes_file(const char* path, uint8_t freepath)
{
    FILE* includePathsFile = fopen(path, "r");
    if (includePathsFile == NULL)
    {
        free((void*)path);
        return;
    }

    char* line = NULL;
    size_t linesize = 0;
    #if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
    while (getlinev2(&line, &linesize, includePathsFile) != EOF)
    #else
    while (getline(&line, &linesize, includePathsFile) != EOF)
    #endif
    {
        line[linesize-1] = 0;
        linesize--;
        baranium_preprocessor_add_include_path(line);
        if (line != NULL)
            free(line);
        line = NULL;
    }
    if (line != NULL)
        free(line);
    fclose(includePathsFile);
    if (freepath)
        free((void*)path);
}

char* get_executable_working_directory(void)
{
    char* result = (char*)malloc(0x1000);

    #if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
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

    argument_parser* parser = argument_parser_init();
    argument_parser_add(parser, Argument_Type_Flag, "-h", "--help");
    argument_parser_add(parser, Argument_Type_Flag, "-d", "--debug");
    argument_parser_add(parser, Argument_Type_Flag, "-v", "--version");
    argument_parser_add(parser, Argument_Type_Flag, "-e", "--export");
    argument_parser_add(parser, Argument_Type_Value, "-l", "--link");
    argument_parser_add(parser, Argument_Type_Value, "-o", "--output");
    argument_parser_add(parser, Argument_Type_Value, "-I", "--include");
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
    FILE* logOutput = fopen("compiler.log", "wb+");
    logSetStream(logOutput);

    char* output = "output.bin";
    uint8_t is_library = 0;

    uint8_t outputPathPresent = argument_parser_has(parser, "-o");
    argument* userIncludes = argument_parser_get(parser, "-I");
    is_library = argument_parser_has(parser, "-e");

    baranium_compiler_context* context = baranium_compiler_context_init();

    // read include paths
    char* executableFilePath = get_executable_working_directory();
    size_t executableFilePathLastSeperatorIndex = str_index_of(executableFilePath, OS_DELIMITER);
    if (executableFilePathLastSeperatorIndex != (size_t)-1)
        executableFilePath[executableFilePathLastSeperatorIndex+1] = 0;

    char* baraniumInclude = getenv("BARANIUM_INCLUDE");
    if (baraniumInclude != NULL)
        baranium_preprocessor_add_include_path(baraniumInclude);
    baranium_preprocessor_add_include_path(stringf("%s../include", executableFilePath));
    baranium_preprocessor_add_include_path(stringf("%sinclude", executableFilePath));
    baranium_preprocessor_add_include_path(executableFilePath);

    // library path
    baranium_compiler_context_set_library_directory(context, stringf("%s%s", executableFilePath, "../lib"));

    char* baraniumIncludeFile = getenv("BARANIUM_INCLUDE_CONFIG");
    if (baraniumIncludeFile != NULL)
        read_includes_file(baraniumIncludeFile, 0);
    read_includes_file(strsubstr(stringf("%s../etc/include.paths", executableFilePath),0,-1), 1);
    read_includes_file(strsubstr(stringf("%s../include.paths", executableFilePath),0,-1), 1);
    read_includes_file(strsubstr(stringf("%setc/include.paths", executableFilePath),0,-1), 1);
    read_includes_file(strsubstr(stringf("%sinclude.paths", executableFilePath),0,-1), 1);

    if (userIncludes != NULL)
        read_includes_file(userIncludes->values[0], 0);

    //////////////////////
    /// Error handling ///
    //////////////////////

    if (outputPathPresent)
    {
        argument* outputArg = argument_parser_get(parser, "-o");
        output = (char*)outputArg->values[0];
    }

    // remove last directory seperator from the output path/file
    if (strlen(output) > 2)
    {
        size_t outputLastSeperatorIndex = str_index_of(output, '/');
        size_t outputLastSeperatorIndex2 = str_index_of(output, '\\');

        if (outputLastSeperatorIndex < outputLastSeperatorIndex2 && outputLastSeperatorIndex2 != (size_t)-1)
            outputLastSeperatorIndex = outputLastSeperatorIndex2;

        if (outputLastSeperatorIndex == strlen(output)-1)
            output[outputLastSeperatorIndex] = 0;
    }

    argument* libraries = argument_parser_get(parser, "-l");
    for (int i = 0; libraries != NULL && i < libraries->value_count; i++)
        baranium_compiler_context_add_library(context, libraries->values[i]);

    /////////////////////////////////////////
    /// Lexing and parsing of the sources ///
    /////////////////////////////////////////

    for (argument* file = parser->unparsed->start; file != NULL; file = file->next)
    {
        char* sourceFileDirectory = (char*)file->values[0];
        size_t sourceFileDirectorySeperatorIndex = str_index_of(sourceFileDirectory, '/');
        size_t sourceFileDirectorySeperatorIndex2 = str_index_of(sourceFileDirectory, '\\');
        if (sourceFileDirectorySeperatorIndex < sourceFileDirectorySeperatorIndex2 && sourceFileDirectorySeperatorIndex2 != (size_t)-1)
            sourceFileDirectorySeperatorIndex = sourceFileDirectorySeperatorIndex2;

        if (sourceFileDirectorySeperatorIndex == (size_t)-1)
            sourceFileDirectory = ".";
        else
            sourceFileDirectory[sourceFileDirectorySeperatorIndex] = 0;
        baranium_preprocessor_add_include_path(sourceFileDirectory);
        if (sourceFileDirectorySeperatorIndex != (size_t)-1)
            sourceFileDirectory[sourceFileDirectorySeperatorIndex] = OS_DELIMITER;

        FILE* inputFile = fopen(file->values[0], "r");
        if (inputFile == NULL)
        {
            LOGERROR(stringf("Error: file '%s' doesn't exist", file->values[0]));
            continue;
        }
        LOGINFO(stringf("Compiling file '%s'...", file->values[0]));
        baranium_compiler_context_add_source(context, inputFile);

        fclose(inputFile);

        baranium_preprocessor_pop_last_include();
    }

    ////////////////////////////////////////
    /// Compiling and writing the binary ///
    ////////////////////////////////////////

    baranium_compiler_context_compile(context, output, is_library);
    baranium_compiler_context_dispose(context);

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
    printf("\t-e\tCompile as a library\n");
    printf("\t-l <name>\tLink against a library named `name`\n");
    printf("\t-I <file>\tSpecify file containing all custom user include directories\n");
    printf("\t-d\t\tPrint debug messages (only useful for debugging the compiler itself!)\n\n");
}
