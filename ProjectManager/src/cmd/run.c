#include <baranium/file_util.h>
#include <baranium/logging.h>
#include <operations.h>
#include <commands.h>
#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <util.h>

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
#   define OS_DELIMITER '\\'
#else
#   define OS_DELIMITER '/'
#endif

void cmd_run(cmd_args_t* userparam)
{
    cmd_args_t args = *userparam;

    config_file_t cfg = {0, 0, 0, 0};
    if (!open_project_file(&cfg))
    {
        LOGERROR("Cannot find project file in current directory");
        return;
    }
    config_property_t* property = config_file_get(&cfg, "build.project_name");
    if (!property)
    {
        config_file_close(&cfg);
        LOGERROR("Invalid project file, missing 'project_name' property");
        return;
    }
    const char* project_name = property->value.stringValue;
    const char* output_directory = "bin";
    property = config_file_get(&cfg, "build.output");
    if (property)
        output_directory = property->value.stringValue;

    if (args.count != 0)
    {
        LOGWARNING("Running with command line arguments is currently unsupported");
        ///
        /// TODO: --- implement ---
        ///
    }

    char* execname_dirname = (char*)executablename+strlen(executablename)-1;
    for (; *execname_dirname != OS_DELIMITER; execname_dirname--);
    execname_dirname[0] = 0;
    const char* cmd = stringf("%s/bar %s/%s", executablename, output_directory, project_name);
    LOGWARNING("cmd: '%s'", cmd);
    system(cmd);
    execname_dirname[0] = OS_DELIMITER;

    config_file_close(&cfg);
}
