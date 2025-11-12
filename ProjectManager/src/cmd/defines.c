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

void cmd_define_add(cmd_args_t* argsptr, config_file_t* cfg)
{
    cmd_args_t args = *argsptr;
    if (args.count < 1)
    {
        LOGERROR("Please specify define name and value (optional)");
        return;
    }
    const char* name = args.values[0];
    const char* value = args.values[1];

    config_property_t* define_property = config_file_add_property(cfg, stringf("defines.%s", name));
    if (args.count > 1)
        config_property_set_value_from_string(define_property, value);

    LOGINFO("Added define '%s'", name);

    save_project_file(cfg);
}

void cmd_define_remove(cmd_args_t* argsptr, config_file_t* cfg)
{
    cmd_args_t args = *argsptr;
    if (args.count != 1)
    {
        LOGERROR("Please only specify define name");
        return;
    }
    const char* name = args.values[0];

    if (!config_file_has_property(cfg, stringf("defines.%s", name)))
    {
        LOGERROR("Define '%s' was not found", name);
        return;
    }

    config_file_remove_property(cfg, stringf("defines.%s", name));
    LOGINFO("Removed define '%s'", name);

    save_project_file(cfg);
}

void cmd_defines(cmd_args_t* userparam)
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
        goto show_defines;

    if (strcmp(command, "add") == 0)
    {
        cmd_define_add(&args2, &cfg);
        config_file_close(&cfg);
        return;
    }
    else if (strcmp(command, "remove") == 0)
    {
        cmd_define_remove(&args2, &cfg);
        config_file_close(&cfg);
        return;
    }

show_defines:
    LOGINFO("Current defines:");
    config_section_t* defines_section = config_file_get_section(&cfg, "defines");
    for (size_t i = 0; i < defines_section->propertyCount; i++)
    {
        const char* value = get_property_value_string(&defines_section->properties[i]);
        LOGINFO("%s = %s", defines_section->properties[i].name, value);
        free((void*)value);
    }

    config_file_close(&cfg);
}
