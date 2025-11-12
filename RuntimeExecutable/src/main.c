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

#define PRINT_VERSION printf("Baranium Runtime Version %d.%d.%d %s\n", BARANIUM_VERSION_YEAR, BARANIUM_VERSION_MONTH, BARANIUM_VERSION_DATE, BARANIUM_VERSION_PHASE)

uint8_t debug_mode_enabled;

extern void setup_callbacks(void);

void print_help_message(void)
{
    printf("bar [options] <filepath>\n");
    printf("Options:\n");
    printf("\t-h/--help:\t\t\tShow this help message\n");
    printf("\t-v/--version:\t\t\tShow the version of the runtime\n");
    printf("\t-stdout/--enable-stdout:\tShow the version of the runtime\n");
    printf("\t-d/--debug:\t\t\tEnable debug messages\n");
}

char* get_executable_working_directory(void)
{
    static char result[0x1000];

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
    int exit_code = 0;
    if (argc < 2)
    {
        print_help_message();
        return exit_code;
    }

    argument_parser_t parser;
    argument_parser_init(&parser);
    argument_parser_add(&parser, ARGUMENT_TYPE_FLAG, "-h", "--help");
    argument_parser_add(&parser, ARGUMENT_TYPE_FLAG, "-d", "--debug");
    argument_parser_add(&parser, ARGUMENT_TYPE_FLAG, "-stdout", "--enable-stdout");
    argument_parser_add(&parser, ARGUMENT_TYPE_FLAG, "-v", "--version");
    argument_parser_parse(&parser, argc, argv);

    if (argument_parser_has(&parser, "-h"))
    {
        print_help_message();
        argument_parser_dispose(&parser);
        return exit_code;
    }

    if (argument_parser_has(&parser, "-v"))
    {
        PRINT_VERSION;
        argument_parser_dispose(&parser);
        return exit_code;
    }

    debug_mode_enabled = argument_parser_has(&parser, "-d");
    if (debug_mode_enabled)
        PRINT_VERSION;
    log_enable_debug_msgs(debug_mode_enabled);
    log_enable_stdout(argument_parser_has(&parser, "-stdout"));
    FILE* logOutput = fopen("runtime.log", "wb+");
    log_set_stream(logOutput);

    if (debug_mode_enabled)
        LOGINFO("Debug messages enabled");

    if (parser.unparsed.size != 1)
    {
        print_help_message();
        LOGERROR("Invalid number of files passed, expected one, got %ld", parser.unparsed.size);
        argument_parser_dispose(&parser);
        fclose(logOutput);
        exit_code = -1;
        return exit_code;
    }

    const char* filePath = parser.unparsed.data[0].values[0];
    argument_parser_dispose(&parser);

    baranium_runtime* runtime = baranium_init_runtime();
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
    if (script == NULL)
    {
        log_enable_stdout(1);
        LOGERROR("Invalid file passed, terminating...");
        exit_code = -2;
        goto end;
    }

    index_t mainIndex = baranium_script_get_id_of(script, "main");

    baranium_function* main = baranium_script_get_function_by_id(script, mainIndex);
    baranium_function_call_data_t args = {
        .data = (baranium_value_t[]){
            (baranium_value_t){.str=""}
        },
        .types = (baranium_variable_type_t[]){
            BARANIUM_VARIABLE_TYPE_STRING
        },
        .count = 0
    };
    baranium_function_call(main, args);
    exit_code = main->return_data.value.snum32;
    baranium_function_dispose(main);

end:
    baranium_close_script(script);
    baranium_close_handle(handle);

    baranium_dispose_runtime(runtime);
    fclose(logOutput);

    return exit_code;
}
