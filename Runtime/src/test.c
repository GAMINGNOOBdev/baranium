#include <baranium/function.h>
#include <baranium/runtime.h>
#include <baranium/script.h>

int main(int argc, const char** argv)
{
    #if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
        printf("f*ck you, your os sucks, it can't even fread into allocated memory without crashing\n");
        printf("i am too lazy to find out why so go f*ck yourself and install linux/wsl to run this shit\n");
        return -0xBAD;
    #endif

    if (argc < 2)
    {
        printf("please provide a compiled script\n");
        return -1;
    }

    BaraniumRuntime* runtime = baranium_init();
    baranium_set_context(runtime);

    BaraniumHandle* handle = baranium_open_handle(argv[1]);
    BaraniumScript* script = baranium_open_script(handle);

    index_t mainIndex = baranium_script_get_id_of(script, "main");
    index_t TestStringIndex = baranium_script_get_id_of(script, "TestString");
    index_t myObjectIndex = baranium_script_get_id_of(script, "myObject");
    printf("index of 'main': %ld, 'TestString': %ld, 'myObject': %ld \n", mainIndex, TestStringIndex, myObjectIndex);

    BaraniumFunction* main = baranium_script_get_function_by_id(script, mainIndex);
    baranium_function_call(runtime, main);
    baranium_function_dispose(main);

    BaraniumField* myObject = baranium_script_get_field_by_id(script, myObject);
    baranium_field_dispose(myObject);

    BaraniumVariable* TestString = baranium_script_get_variable(script, "TestString");
    baranium_variable_dispose(TestString);

    baranium_close_script(script);
    baranium_close_handle(handle);

    baranium_cleanup(runtime);

    return 0;
}