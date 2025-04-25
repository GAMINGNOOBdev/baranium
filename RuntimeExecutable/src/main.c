#include <baranium/backend/varmath.h>
#include <baranium/variable.h>
#include <baranium/function.h>
#include <baranium/callback.h>
#include <baranium/defines.h>
#include <baranium/logging.h>
#include <baranium/runtime.h>
#include <baranium/script.h>
#include <argument_parser.h>
#include <string.h>
#include <stdio.h>

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_APPLE
#   include <stdlib.h>
#else
#   include <malloc.h>
#endif

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
#   include <Windows.h>
#   undef max
#   undef min
#   define OS_DELIMITER '\\'
#else
#   include <sys/stat.h>
#   include <unistd.h>
#   define OS_DELIMITER '/'
#endif

#define PRINT_VERSION printf("Baranium runtime version %d.%d.%d %s\n", BARANIUM_VERSION_YEAR, BARANIUM_VERSION_MONTH, BARANIUM_VERSION_DATE, BARANIUM_VERSION_PHASE)

uint8_t debug_mode_enabled;

void print_help_message(void);
extern void setup_callbacks(void);

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

int main(int argc, const char** argv)
{
    if (argc < 2)
    {
        print_help_message();
        return 0;
    }

    argument_parser* parser = argument_parser_init();
    argument_parser_add(parser, Argument_Type_Flag, "-h", "--help");
    argument_parser_add(parser, Argument_Type_Flag, "-d", "--debug");
    argument_parser_add(parser, Argument_Type_Flag, "-v", "--version");
    argument_parser_parse(parser, argc, argv);

    if (argument_parser_has(parser, "-h"))
    {
        print_help_message();
        argument_parser_dispose(parser);
        return 0;
    }

    if (argument_parser_has(parser, "-v"))
    {
        PRINT_VERSION;
        argument_parser_dispose(parser);
        return 0;
    }

    debug_mode_enabled = argument_parser_has(parser, "-d");
    if (debug_mode_enabled)
        PRINT_VERSION;
    logEnableDebugMsgs(debug_mode_enabled);
    logEnableStdout(!debug_mode_enabled);
    FILE* logOutput = fopen("runtime.log", "wb+");
    logSetStream(logOutput);

    if (debug_mode_enabled)
        LOGINFO("Debug messages enabled");

    if (parser->unparsed->size != 1)
    {
        print_help_message();
        LOGERROR(stringf("Invalid number of files passed, expected one, got %ld", parser->unparsed->size));
        argument_parser_dispose(parser);
        fclose(logOutput);
        return -1;
    }

    const char* filePath = parser->unparsed->start->values[0];
    argument_parser_dispose(parser);

    baranium_runtime* runtime = baranium_init();
    baranium_set_runtime(runtime);

    char* executableFilePath = get_executable_working_directory();
    size_t executableFilePathLastSeperatorIndex = strlen(executableFilePath)-1;
    while (executableFilePath[executableFilePathLastSeperatorIndex] != OS_DELIMITER)
        executableFilePathLastSeperatorIndex--;
    if (executableFilePathLastSeperatorIndex != 0)
        executableFilePath[executableFilePathLastSeperatorIndex+1] = 0;
    baranium_runtime_set_library_path(stringf("%s../lib", executableFilePath));

    setup_callbacks();

    baranium_handle* handle = baranium_open_handle(filePath);
    baranium_script* script = baranium_open_script(handle);

    index_t mainIndex = baranium_script_get_id_of(script, "main");

    baranium_function* main = baranium_script_get_function_by_id(script, mainIndex);
    baranium_function_call(main, NULL, NULL, 0);
    baranium_function_dispose(main);

    baranium_close_script(script);
    baranium_close_handle(handle);

    baranium_dispose_runtime(runtime);
    free(executableFilePath);
    fclose(logOutput);

    return 0;
}

void print_help_message(void)
{
    printf("bar [options] <filepath>\n");
    printf("Options:\n");
    printf("\t-h/--help:\tShow this help message\n");
    printf("\t-v/--version:\tShow the version of the runtime\n");
    printf("\t-d/--debug:\tEnable debug messages\n");
}
