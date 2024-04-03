#include <baranium/runtime.h>
#include <baranium/script.h>

int main(int argc, const char** argv)
{
    #if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
        printf("f*ck you, your os sucks, it can't even fread into allocated memory without crashing\n");
        printf("i am too lazy to find out why so go f*ck yourself and install linux/wsl to run this shit\n");
        return -0xBAD;
    #endif

    // printf("don't execute atm\n");
    // return 0;

    if (argc < 2)
    {
        printf("please provide a compiled script\n");
        return -1;
    }

    BaraniumRuntime* runtime = baranium_init();
    baranium_set_context(runtime);

    BaraniumHandle* handle = baranium_open_handle(argv[1]);
    BaraniumScript* script = baranium_open_script(handle);

    index_t mainIndex = baranium_script_get_location_of(script, "main");
    printf("index of \"main\": %ld\n", mainIndex);

    baranium_close_script(script);
    baranium_close_handle(handle);

    baranium_cleanup(runtime);

    return 0;
}