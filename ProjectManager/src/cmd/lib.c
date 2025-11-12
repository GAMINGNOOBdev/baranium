#include <baranium/file_util.h>
#include <baranium/logging.h>
#include <operations.h>
#include <commands.h>
#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <util.h>

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
#   define OS_DELIMITER '\\'
#else
#   define OS_DELIMITER '/'
#endif

void cmd_library_add(cmd_args_t* argsptr, config_file_t* cfg)
{
    cmd_args_t args = *argsptr;
    if (args.count != 2)
    {
        LOGERROR("Please specify library name and git link");
        return;
    }
    const char* name = args.values[0];
    const char* source = args.values[1];

    config_property_t* library_property = config_file_add_property(cfg, stringf("libraries.%s", name));
    config_property_set_value_from_string(library_property, source);

    LOGINFO("Added library '%s'", name);

    save_project_file(cfg);
}

void cmd_library_remove(cmd_args_t* argsptr, config_file_t* cfg)
{
    cmd_args_t args = *argsptr;
    if (args.count != 1)
    {
        LOGERROR("Please only specify library name");
        return;
    }
    const char* name = args.values[0];

    if (!config_file_has_property(cfg, stringf("libraries.%s", name)))
    {
        LOGERROR("Library '%s' was not found", name);
        return;
    }

    config_file_remove_property(cfg, stringf("libraries.%s", name));
    LOGINFO("Removed library '%s'", name);

    save_project_file(cfg);
}

void cmd_library(cmd_args_t* userparam)
{
    cmd_args_t args = *userparam;
    if (args.count == 0)
        LOGINFO("Specify what to do, aka `add` or `remove`");

    const char* command = args.values[0];
    cmd_args_t args2 = {args.count-1, args.values+1};

    config_file_t cfg = {0, 0, 0, 0};
    if (!open_project_file(&cfg))
    {
        LOGERROR("Cannot find project file in current directory");
        return;
    }

    if (args.count == 0)
        goto show_libraries;

    if (strcmp(command, "add") == 0)
    {
        cmd_library_add(&args2, &cfg);
        config_file_close(&cfg);
        return;
    }
    else if (strcmp(command, "remove") == 0)
    {
        cmd_library_remove(&args2, &cfg);
        config_file_close(&cfg);
        return;
    }
    else
        LOGERROR("Invalid command '%s'\n", command);

show_libraries:
    LOGINFO("Current libraries:");
    config_section_t* libraries_section = config_file_get_section(&cfg, "libraries");
    for (size_t i = 0; i < libraries_section->propertyCount; i++)
    {
        const char* value = get_property_value_string(&libraries_section->properties[i]);
        LOGINFO("%s at %s", libraries_section->properties[i].name, value);
        free((void*)value);
    }

    config_file_close(&cfg);
}
