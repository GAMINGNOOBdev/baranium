#include <baranium/function.h>
#include <baranium/logging.h>
#include <baranium/runtime.h>
#include <baranium/script.h>
#include <argument_parser.h>

uint8_t debug_mode_enabled;

void print_help_message();

int main(int argc, const char** argv)
{
    ArgumentParser* parser = argument_parser_init();
    argument_parser_add(parser, ArgumentType_Flag, "-h", "--help");
    argument_parser_add(parser, ArgumentType_Flag, "-d", "--debug");
    argument_parser_add(parser, ArgumentType_Flag, "-v", "--version");
    argument_parser_parse(parser, argc, argv);

    if (argument_parser_has(parser, "-v"))
    {
        printf("Baranium runtime version %d.%d %s\n", BARANIUM_VERSION_MAJOR, BARANIUM_VERSION_MINOR, BARANIUM_VERSION_PHASE);
        argument_parser_dispose(parser);
        return 0;
    }

    if (argument_parser_has(parser, "-h"))
    {
        print_help_message();
        return 0;
    }

    debug_mode_enabled = argument_parser_has(parser, "-d");
    logEnableDebugMsgs(debug_mode_enabled);
    FILE* logOutput = fopen("runtime.log", "wb+");
    logSetStream(logOutput);

    if (parser->unparsed->size != 1)
    {
        LOGERROR(stringf("Invalid number of files passed, expected one, got %ld", parser->unparsed->size));
        argument_parser_dispose(parser);
        fclose(logOutput);
        return -1;
    }

    const char* filePath = parser->unparsed->start->Value;
    argument_parser_dispose(parser);

    BaraniumRuntime* runtime = baranium_init();
    baranium_set_context(runtime);

    BaraniumHandle* handle = baranium_open_handle(filePath);
    BaraniumScript* script = baranium_open_script(handle);

    index_t mainIndex = baranium_script_get_id_of(script, "main");

    BaraniumFunction* main = baranium_script_get_function_by_id(script, mainIndex);
    baranium_function_call(runtime, main);
    baranium_function_dispose(main);

    baranium_close_script(script);
    baranium_close_handle(handle);

    baranium_cleanup(runtime);
    fclose(logOutput);

    return 0;
}

void print_help_message()
{
    printf("bgs [options] <filepath>\n");
    printf("Options:\n");
    printf("\t-h/--help:\tShow this help message\n");
    printf("\t-v/--version:\tShow the version of the runtime\n");
    printf("\t-d/--debug:\tEnable debug messages\n");
}