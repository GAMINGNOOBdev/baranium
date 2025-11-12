#include <baranium/file_util.h>
#include <baranium/logging.h>
#include <operations.h>
#include <commands.h>
#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <util.h>

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
#   define OS_DELIMITER '\\'
#else
#   define OS_DELIMITER '/'
#endif

void cmd_create(cmd_args_t* userparam)
{
    cmd_args_t args = *userparam;
    if (args.count < 0 || args.count > 2)
    {
        LOGERROR("No project name given for `create` command");
        return;
    }

    const char* project_name = args.values[0];
    const char* target = args.count > 1 ? args.values[1] : project_name;
    FILE* config_file = fopen(stringf("%s/barproject.toml", target), "r");
    if (config_file != NULL)
    {
        fclose(config_file);
        LOGERROR("Already existing project file in '%s', can't create project", target);
        return;
    }

    char* cwd = get_current_working_directory();
    if (!baranium_file_util_directory_exists(stringf("%s/%s", cwd, target)))
        baranium_file_util_create_directory(stringf("%s/%s", cwd, target));

    config_file = fopen(stringf("%s/%s/barproject.toml", cwd, target), "wb+");
    config_file_t cfg = {0, 0, 0, 0};
    config_file_add_section(&cfg, "libraries");
    config_file_add_section(&cfg, "defines");
    config_file_add_section(&cfg, "build");
    config_property_t* project_name_property = config_file_add_property(&cfg, "build.project_name");
    config_property_set_string(project_name_property, project_name);
    config_file_save(&cfg, config_file);
    config_file_close(&cfg);
    fclose(config_file);

    baranium_file_util_create_directory(stringf("%s/%s/src", cwd, target));
}
