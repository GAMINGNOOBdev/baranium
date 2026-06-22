#include <baranium/file_util.h>
#include <baranium/logging.h>
#include <operations.h>
#include <commands.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <toml.h>
#include <util.h>

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
#   define OS_DELIMITER '\\'
#else
#   define OS_DELIMITER '/'
#endif

void cmd_define_add(cmd_args_t* argsptr, toml_section* cfg)
{
    cmd_args_t args = *argsptr;
    if (args.count < 1)
    {
        LOGERROR("Please specify define name and value (optional)");
        return;
    }
    const char* name = args.values[0];
    const char* value = args.values[1];

    toml_property* define_property = toml_section_add_property(cfg, stringf("defines.%s", name));
    if (args.count > 1)
        toml_property_set_value_from_string(define_property, value);

    LOGINFO("Added define '%s'", name);

    save_project_file(cfg);
}

void cmd_define_remove(cmd_args_t* argsptr, toml_section* cfg)
{
    cmd_args_t args = *argsptr;
    if (args.count != 1)
    {
        LOGERROR("Please only specify define name");
        return;
    }
    const char* name = args.values[0];

    if (!toml_section_has_property(cfg, stringf("defines.%s", name)))
    {
        LOGERROR("Define '%s' was not found", name);
        return;
    }

    toml_section_remove_property(cfg, stringf("defines.%s", name));
    LOGINFO("Removed define '%s'", name);

    save_project_file(cfg);
}

void cmd_defines(cmd_args_t* userparam)
{
    cmd_args_t args = *userparam;
    if (args.count == 0)
        LOGINFO("Specify what to do, aka `add` or `remove`");

    toml_section cfg = TOML_SECTION_EMPTY;
    if (!open_project_file(&cfg))
    {
        LOGERROR("Cannot find project file in current directory");
        return;
    }
    if (args.count == 0)
        goto show_defines;

    toml_section* defines_section = NULL;
    const char* command = args.values[0];
    cmd_args_t args2 = {args.count-1, args.values+1};

    if (strcmp(command, "add") == 0)
    {
        cmd_define_add(&args2, &cfg);
        toml_section_dispose(&cfg);
        return;
    }
    else if (strcmp(command, "remove") == 0)
    {
        cmd_define_remove(&args2, &cfg);
        toml_section_dispose(&cfg);
        return;
    }

show_defines:
    defines_section = toml_section_get_section(&cfg, "defines");

    if (defines_section)
        LOGINFO("Current defines:");

    for (size_t i = 0; defines_section && i < defines_section->propertyCount; i++)
    {
        const char* value = get_property_value_string(&defines_section->properties[i]);
        LOGINFO("'%s' = %s", defines_section->properties[i].name, value);
        free((void*)value);
    }

    toml_section_dispose(&cfg);
}
