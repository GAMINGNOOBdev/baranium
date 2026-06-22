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

struct toml_property_pair
{
    const char* name;
    const char* description;
    int type;
    cmd_args_t(*handle)(struct toml_property_pair pair, cmd_args_t params, toml_section* config);
};

cmd_args_t toml_handle_value_option_pair(struct toml_property_pair pair, cmd_args_t params, toml_section* config)
{
    cmd_args_t endargs = {
        params.count-1,
        params.values+1
    };

    toml_property* property = toml_section_get(config, stringf("build.%s", pair.name));
    if (params.count == 0)
    {
        const char* value = get_property_value_string(property);
        LOGINFO("%s = %s", pair.name, value);
        free((void*)value);
        return EMPTY_CMD_ARGS;
    }
    const char* value = params.values[0];
    int type = toml_estimate_value_type(value);

    if (strcmp(value, "default") == 0)
    {
        toml_section_remove_property(config, stringf("build.%s", pair.name));
        LOGINFO("Set 'build.%s' to default value", pair.name);
        return endargs;
    }
    if (type != pair.type)
    {
        LOGERROR("Invalid value type for '%s', expected %s, got %s", pair.name, toml_property_value_type_as_string(pair.type), toml_property_value_type_as_string(type));
        return EMPTY_CMD_ARGS;
    }

    if (property == NULL)
        property = toml_section_add_property(config, stringf("build.%s", pair.name));

    endargs.count--;
    endargs.values++;

    toml_property_set_value_from_string(property, value);
    char* valuestr = (char*)get_property_value_string(property);
    LOGINFO("Successfully set %s to %s", pair.name, valuestr);
    free(valuestr);

    return endargs;
}

cmd_args_t toml_handle_array_option_pair(struct toml_property_pair pair, cmd_args_t params, toml_section* config)
{
    cmd_args_t endargs = {
        params.count-1,
        params.values+1
    };

    toml_property* property = toml_section_get(config, stringf("build.%s", pair.name));
    if (params.count == 0)
    {
        LOGINFO("Missing Arguments: <add/remove/default> <value/index>");
        const char* value = get_property_value_string(property);
        LOGINFO("%s = %s", pair.name, value);
        free((void*)value);
        return EMPTY_CMD_ARGS;
    }
    const char* command = params.values[0];

    if (strcmp(command, "default") == 0)
    {
        toml_section_remove_property(config, stringf("build.%s", pair.name));
        LOGINFO("Set %s to default value", pair.name);
        return endargs;
    }

    if (params.count != 2)
    {
        LOGERROR("No parameters specified for command '%s'", command);
        return EMPTY_CMD_ARGS;
    }

    if (property == NULL)
    {
        property = toml_section_add_property(config, stringf("build.%s", pair.name));
        toml_property_set_value_from_string(property, "[]");
    }

    const char* value = params.values[1];
    if (strcmp(command, "add") == 0)
    {
        toml_property tmp = TOML_PROPERTY_EMPTY;
        toml_property_set_value_from_string(&tmp, value);
        toml_property_array_add(property, &tmp);
        toml_property_dispose(&tmp);
        LOGINFO("Successfully added '%s' to %s", value, pair.name);

        endargs.count--;
        endargs.values++;
        return endargs;
    }
    else if (strcmp(command, "remove") == 0)
    {
        if (toml_estimate_value_type(value) != TOML_PROPERTY_TYPE_INT)
        {
            LOGERROR("Expected integer index for 'remove' command");
            return EMPTY_CMD_ARGS;
        }

        char* tmp;
        int index = strtoll(value, &tmp, 10);
        if (index < 0 || index >= property->arrayLength)
        {
            LOGERROR("Index %d is not in range for %s", index, pair.name);
            return EMPTY_CMD_ARGS;
        }

        toml_property_array_remove(property, index);
        LOGINFO("Removed element %d of %s", index, pair.name);

        endargs.count--;
        endargs.values++;
        return endargs;
    }

    LOGERROR("Invalid command %s for property %s of type %s", command, pair.name, toml_property_value_type_as_string(pair.type));
    return EMPTY_CMD_ARGS;
}

static struct toml_property_pair toml_properties[] = {
    {.name = "project_name", .description="Project name", .type=TOML_PROPERTY_TYPE_STRING, .handle=toml_handle_value_option_pair},
    {.name = "library", .description="Whether this is a library build or not", .type=TOML_PROPERTY_TYPE_BOOL, .handle=toml_handle_value_option_pair},
    {.name = "include", .description="Local include directory", .type=TOML_PROPERTY_TYPE_STRING, .handle=toml_handle_value_option_pair},
    {.name = "output", .description="Output directory", .type=TOML_PROPERTY_TYPE_STRING, .handle=toml_handle_value_option_pair},
    {.name = "prebuild_commands", .description="Commands to execute before building the baranium project (string array)", .type=TOML_PROPERTY_TYPE_ARRAY, .handle=toml_handle_array_option_pair},
};

cmd_args_t cmd_config(cmd_args_t* userparam)
{
    cmd_args_t args = *userparam;
    if (args.count <= 0)
    {
        LOGERROR("Specify what property to change and it's value");
        LOGINFO("to set values back to the defaults just use 'default' instead of the proper value type");
        LOGINFO("Valid config options:");
        for (size_t i = 0; i < sizeof(toml_properties) / sizeof(struct toml_property_pair); i++)
            LOGINFO("\t%s - %s\t%s", toml_properties[i].name, toml_property_value_type_as_string(toml_properties[i].type), toml_properties[i].description);

        toml_section cfg = TOML_SECTION_EMPTY;
        if (!open_project_file(&cfg))
            return EMPTY_CMD_ARGS;

        LOGINFO("Current configs:");
        toml_section* build_section = toml_section_get_section(&cfg, "build");
        for (size_t i = 0; i < build_section->propertyCount; i++)
        {
            const char* value = get_property_value_string(&build_section->properties[i]);
            LOGINFO("build.%s = %s", build_section->properties[i].name, value);
            free((void*)value);
        }

        toml_section_dispose(&cfg);
        return EMPTY_CMD_ARGS;
    }

    const char* name = args.values[0];
    int index = -1;
    for (size_t i = 0; i < sizeof(toml_properties) / sizeof(struct toml_property_pair); i++)
    {
        if (strcmp(toml_properties[i].name, name) != 0)
            continue;

        index = i;
        break;
    }
    if (index == -1)
    {
        LOGERROR("%s is not a valid config option", name);
        LOGINFO("Valid config options:");
        for (size_t i = 0; i < sizeof(toml_properties) / sizeof(struct toml_property_pair); i++)
            LOGINFO("\t%s - %s\t%s", toml_properties[i].name, toml_property_value_type_as_string(toml_properties[i].type), toml_properties[i].description);
        return EMPTY_CMD_ARGS;
    }

    toml_section cfg = TOML_SECTION_EMPTY;
    if (!open_project_file(&cfg))
    {
        LOGERROR("Cannot find project file in current directory");
        return EMPTY_CMD_ARGS;
    }

    cmd_args_t params = {
        .count=args.count-1,
        .values=args.values+1
    };
    cmd_args_t endargs = toml_properties[index].handle(toml_properties[index], params, &cfg);

    save_project_file(&cfg);
    toml_section_dispose(&cfg);

    return endargs;
}
