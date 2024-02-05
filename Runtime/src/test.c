#include <baranium/runtime.h>

int main(int argc, const char** argv)
{
    BaraniumRuntime* runtime = baranium_init();
    baranium_set_context(runtime);

    BaraniumHandle* handle = baranium_open_handle("build_run.sh");
    baranium_close_handle(handle);

    baranium_cleanup(runtime);
}