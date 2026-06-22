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

void cmd_library_add(cmd_args_t* argsptr, toml_section* cfg)
{
    cmd_args_t args = *argsptr;
    if (args.count != 2)
    {
        LOGERROR("Please specify library name and git link");
        return;
    }
    const char* name = args.values[0];
    const char* source = args.values[1];

    int type = toml_estimate_value_type(source);
    if (type != TOML_PROPERTY_TYPE_STRING && type != TOML_PROPERTY_TYPE_UNKNOWN)
    {
        LOGERROR("Invalid source");
        return;
    }

    toml_property* library_property = toml_section_add_property(cfg, stringf("libraries.%s", name));
    toml_property_set_value_from_string(library_property, source);

    LOGINFO("Added library '%s' at remote '%s'", name, source);

    save_project_file(cfg);
}

void cmd_library_remove(cmd_args_t* argsptr, toml_section* cfg)
{
    cmd_args_t args = *argsptr;
    if (args.count != 1)
    {
        LOGERROR("Please specify library name");
        return;
    }
    const char* name = args.values[0];

    if (!toml_section_has_property(cfg, stringf("libraries.%s", name)))
    {
        LOGERROR("Library '%s' was not found", name);
        return;
    }

    toml_section_remove_property(cfg, stringf("libraries.%s", name));
    LOGINFO("Removed library '%s'", name);

    save_project_file(cfg);
}

void cmd_library(cmd_args_t* userparam)
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
        goto show_libraries;

    const char* command = args.values[0];
    toml_section* libraries_section = NULL;
    cmd_args_t args2 = {args.count-1, args.values+1};

    if (strcmp(command, "add") == 0)
    {
        cmd_library_add(&args2, &cfg);
        toml_section_dispose(&cfg);
        return;
    }
    else if (strcmp(command, "remove") == 0)
        cmd_library_remove(&args2, &cfg);
    else
        LOGERROR("Invalid command '%s'\n", command);

show_libraries:
    libraries_section = toml_section_get_section(&cfg, "libraries");

    if (libraries_section)
        LOGINFO("Current libraries:");

    for (size_t i = 0; libraries_section && i < libraries_section->propertyCount; i++)
    {
        const char* value = get_property_value_string(&libraries_section->properties[i]);
        LOGINFO("'%s' at %s", libraries_section->properties[i].name, value);
        free((void*)value);
    }

    toml_section_dispose(&cfg);
}
