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

int cmd_install_status = 0;
const char* cmd_install_output_path;
const char* cmd_install_output_target_path;
void cmd_install_copy_file(const char* path)
{
    if (cmd_install_status != 0)
        return;

    const char* input = strdup(stringf("%s%c%s", cmd_install_output_path, OS_DELIMITER, path));
    const char* output = strdup(stringf("%s%c%s", cmd_install_output_target_path, OS_DELIMITER, path));
    cmd_install_status = copy(input, output);
    LOGINFO("Copying file '%s' to '%s'", input, output);
    free((void*)input);
    free((void*)output);
}

void cmd_install(cmd_args_t* userparam)
{
    cmd_args_t args = *userparam;
    if (args.count != 0)
        LOGINFO("`build` commands takes no arguments");

    config_file_t cfg = {0, 0, 0, 0};
    if (!open_project_file(&cfg))
    {
        LOGERROR("Cannot find project file in current directory");
        return;
    }

    config_property_t* property = config_file_get(&cfg, "build.library");
    int is_library = (property ? (property->value.boolValue ? 1 : 0) : 0);
    property = config_file_get(&cfg, "build.project_name");
    if (property == NULL)
    {
        LOGERROR("Invalid project file, could not find 'build.project_name' config property");
        config_file_close(&cfg);
        return;
    }
    const char* project_name = property->value.stringValue;

    if (!is_library)
    {
        LOGWARNING("Project '%s' is not a library, skipping installation.", property->value.stringValue);
        config_file_close(&cfg);
        return;
    }

    cmd_install_output_path = "bin";
    property = config_file_get(&cfg, "build.output");
    if (property)
        cmd_install_output_path = property->value.stringValue;

    char* execname_dirname = (char*)executablename+strlen(executablename)-1;
    for (; *execname_dirname != OS_DELIMITER; execname_dirname--); // first one is to remove the executable name
    execname_dirname--;
    for (; *execname_dirname != OS_DELIMITER; execname_dirname--); // second one to remove the directory name (should always be bin)
    execname_dirname[0] = 0;
    cmd_install_output_target_path = strdup(stringf("%s%clib", executablename, OS_DELIMITER));
    execname_dirname[0] = OS_DELIMITER;

    if (!baranium_file_util_directory_exists(cmd_install_output_target_path))
        baranium_file_util_create_directory(cmd_install_output_target_path);
    baranium_file_util_iterate_directory(cmd_install_output_path, BARANIUM_FILE_UTIL_FILTER_MASK_ALL_FILES, cmd_install_copy_file);
    if (cmd_install_status == 0)
        LOGINFO("successfully installed library '%s' to '%s'", project_name, cmd_install_output_target_path);
    else if (cmd_install_status == 1)
        LOGERROR("cannot create output files for library '%s' at '%s'", project_name, cmd_install_output_target_path);
    else if (cmd_install_status == -1)
        LOGERROR("cannot find built files for library '%s'", project_name);

    free((void*)cmd_install_output_target_path);
    config_file_close(&cfg);
}
