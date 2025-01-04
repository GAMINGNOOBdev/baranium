#include "baranium/defines.h"
#include <baranium/backend/varmath.h>
#include <baranium/variable.h>
#include <baranium/function.h>
#include <baranium/callback.h>
#include <baranium/logging.h>
#include <baranium/runtime.h>
#include <baranium/script.h>
#include <argument_parser.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>

uint8_t debug_mode_enabled;

void print_help_message();
extern void setup_callbacks();

int main(int argc, const char** argv)
{
    if (argc < 2)
    {
        print_help_message();
        return 0;
    }

    ArgumentParser* parser = argument_parser_init();
    argument_parser_add(parser, ArgumentType_Flag, "-h", "--help");
    argument_parser_add(parser, ArgumentType_Flag, "-d", "--debug");
    argument_parser_add(parser, ArgumentType_Flag, "-v", "--version");
    argument_parser_parse(parser, argc, argv);

    if (argument_parser_has(parser, "-h"))
    {
        print_help_message();
        argument_parser_dispose(parser);
        return 0;
    }

    if (argument_parser_has(parser, "-v"))
    {
        printf("Baranium runtime version %d.%d.%d %s\n", BARANIUM_VERSION_YEAR, BARANIUM_VERSION_MONTH, BARANIUM_VERSION_DATE, BARANIUM_VERSION_PHASE);
        argument_parser_dispose(parser);
        return 0;
    }

    debug_mode_enabled = argument_parser_has(parser, "-d");
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

    const char* filePath = parser->unparsed->start->Value;
    argument_parser_dispose(parser);

    baranium_runtime* runtime = baranium_init();
    baranium_set_context(runtime);

    setup_callbacks();

    baranium_handle* handle = baranium_open_handle(filePath);
    baranium_script* script = baranium_open_script(handle);

    baranium_field* testFunctionResult = baranium_script_get_field(script, "testFunctionResult");
    float val = 100.5f;
    baranium_field_set_value(testFunctionResult, &val, sizeof(float), VARIABLE_TYPE_FLOAT);

    index_t mainIndex = baranium_script_get_id_of(script, "main");

    baranium_function* main = baranium_script_get_function_by_id(script, mainIndex);
    baranium_function_call(runtime, main, NULL, NULL, 0);
    baranium_function_dispose(main);

    baranium_close_script(script);
    baranium_close_handle(handle);

    baranium_cleanup(runtime);
    fclose(logOutput);

    return 0;
}

void print_help_message()
{
    printf("bar [options] <filepath>\n");
    printf("Options:\n");
    printf("\t-h/--help:\tShow this help message\n");
    printf("\t-v/--version:\tShow the version of the runtime\n");
    printf("\t-d/--debug:\tEnable debug messages\n");
}
