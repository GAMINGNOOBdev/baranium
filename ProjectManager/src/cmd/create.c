#include <baranium/file_util.h>
#include <baranium/logging.h>
#include <operations.h>
#include <commands.h>
#include <stdlib.h>
#include <stdio.h>
#include <toml.h>
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
    FILE* toml_file = fopen(stringf("%s/barproject.toml", target), "r");
    if (toml_file != NULL)
    {
        fclose(toml_file);
        LOGERROR("Already existing project file in '%s', can't create project", target);
        return;
    }

    char* cwd = get_current_working_directory();
    if (!baranium_file_util_directory_exists(stringf("%s/%s", cwd, target)))
        baranium_file_util_create_directory(stringf("%s/%s", cwd, target));

    toml_file = fopen(stringf("%s/%s/barproject.toml", cwd, target), "wb+");
    toml_file_t cfg = {0, 0, 0, 0};
    toml_file_add_section(&cfg, "libraries");
    toml_file_add_section(&cfg, "defines");
    toml_file_add_section(&cfg, "build");
    toml_property_t* project_name_property = toml_file_add_property(&cfg, "build.project_name");
    toml_property_set_string(project_name_property, project_name);
    toml_file_save(&cfg, toml_file);
    toml_file_close(&cfg);
    fclose(toml_file);

    baranium_file_util_create_directory(stringf("%s/%s/src", cwd, target));
}
