#include <baranium/function.h>
#include <baranium/logging.h>
#include <baranium/runtime.h>
#include <baranium/script.h>

int main(int argc, const char** argv)
{
    if (argc < 2)
    {
        LOGERROR("please provide one compiled script");
        return -1;
    }

    if (argc > 2)
    {
        LOGERROR("I SAID *ONE* COMPILED SCRIPT!!!!!!");
        return -('w' + 't' + 'f');
    }

    BaraniumRuntime* runtime = baranium_init();
    baranium_set_context(runtime);

    BaraniumHandle* handle = baranium_open_handle(argv[1]);
    BaraniumScript* script = baranium_open_script(handle);

    index_t mainIndex = baranium_script_get_id_of(script, "main");
    LOGINFO(stringf("index of 'main': %ld", mainIndex));

    BaraniumFunction* main = baranium_script_get_function_by_id(script, mainIndex);
    baranium_function_call(runtime, main);
    baranium_function_dispose(main);

    baranium_close_script(script);
    baranium_close_handle(handle);

    baranium_cleanup(runtime);

    return 0;
}