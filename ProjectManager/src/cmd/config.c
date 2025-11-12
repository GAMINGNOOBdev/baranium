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

struct config_property_pair
{
    const char* name;
    const char* description;
    int type;
    void(*handle)(struct config_property_pair pair, cmd_args_t params, config_file_t* config);
};

void config_handle_value_option_pair(struct config_property_pair pair, cmd_args_t params, config_file_t* config)
{
    config_property_t* property = config_file_get(config, stringf("build.%s", pair.name));
    if (params.count == 0)
    {
        const char* value = get_property_value_string(property);
        LOGINFO("%s = %s", pair.name, value);
        free((void*)value);
        return;
    }
    const char* value = params.values[0];
    int type = config_estimate_value_type(value);

    if (strcmp(value, "default") == 0)
    {
        config_file_remove_property(config, stringf("build.%s", pair.name));
        LOGINFO("Set %s to default value", pair.name);
        return;
    }
    if (type != pair.type)
    {
        LOGERROR("Invalid value type for %s, expected %s, got %s", pair.name, config_property_value_type_as_string(pair.type), config_property_value_type_as_string(type));
        return;
    }

    if (property == NULL)
        property = config_file_add_property(config, stringf("build.%s", pair.name));

    config_property_set_value_from_string(property, value);
    char* valuestr = (char*)get_property_value_string(property);
    LOGINFO("Successfully set %s to %s", pair.name, valuestr);
    free(valuestr);
}

void config_handle_array_option_pair(struct config_property_pair pair, cmd_args_t params, config_file_t* config)
{
    config_property_t* property = config_file_get(config, stringf("build.%s", pair.name));
    if (params.count == 0)
    {
        const char* value = get_property_value_string(property);
        LOGINFO("%s = %s", pair.name, value);
        free((void*)value);
        return;
    }
    const char* command = params.values[0];

    if (strcmp(command, "default") == 0)
    {
        config_file_remove_property(config, stringf("build.%s", pair.name));
        LOGINFO("Set %s to default value", pair.name);
        return;
    }

    if (params.count != 2)
    {
        LOGERROR("No parameters specified for command '%s'", command);
        return;
    }

    if (property == NULL)
    {
        property = config_file_add_property(config, stringf("build.%s", pair.name));
        config_property_set_value_from_string(property, "[]");
    }

    const char* value = params.values[1];
    if (strcmp(command, "add") == 0)
    {
        config_property_t tmp = CONFIG_PROPERTY_EMPTY;
        config_property_set_value_from_string(&tmp, value);
        config_property_array_add(property, &tmp);
        config_property_dispose(&tmp);
        LOGINFO("Successfully added '%s' to %s", value, pair.name);
    }
    else if (strcmp(command, "remove") == 0)
    {
        if (config_estimate_value_type(value) != CONFIG_PROPERTY_VALUE_TYPE_INT)
        {
            LOGERROR("Expected integer index for 'remove' command");
            return;
        }

        char* tmp;
        int index = strtoll(value, &tmp, 10);
        int actualIndex = index - 1;
        if (actualIndex < 0 || actualIndex >= property->arrayLength)
        {
            LOGERROR("Index %d is not in range for %s", index, pair.name);
            return;
        }

        config_property_array_remove(property, actualIndex);
        LOGINFO("Removed element %d of %s", index, pair.name);
    }
    else
        LOGERROR("Invalid command %s for property %s of type %s", command, pair.name, config_property_value_type_as_string(pair.type));
}

static struct config_property_pair config_properties[] = {
    {.name = "project_name", .description="Project name", .type=CONFIG_PROPERTY_VALUE_TYPE_STRING, .handle=config_handle_value_option_pair},
    {.name = "library", .description="Whether this is a library build or not", .type=CONFIG_PROPERTY_VALUE_TYPE_BOOL, .handle=config_handle_value_option_pair},
    {.name = "include", .description="Local include directory", .type=CONFIG_PROPERTY_VALUE_TYPE_STRING, .handle=config_handle_value_option_pair},
    {.name = "output", .description="Output directory", .type=CONFIG_PROPERTY_VALUE_TYPE_STRING, .handle=config_handle_value_option_pair},
    {.name = "prebuild_commands", .description="Commands to execute before building the baranium project (string array)", .type=CONFIG_PROPERTY_VALUE_TYPE_ARRAY, .handle=config_handle_array_option_pair},
};

void cmd_config(cmd_args_t* userparam)
{
    cmd_args_t args = *userparam;
    if (args.count == 0)
    {
        LOGERROR("Specify what property to change and it's value");
        LOGINFO("Valid config options:");
        for (size_t i = 0; i < sizeof(config_properties) / sizeof(struct config_property_pair); i++)
            LOGINFO("\t%s - %s\t%s", config_properties[i].name, config_property_value_type_as_string(config_properties[i].type), config_properties[i].description);

        config_file_t cfg = {0, 0, 0, 0};
        if (!open_project_file(&cfg))
            return;

        LOGINFO("Current configs:");
        config_section_t* build_section = config_file_get_section(&cfg, "build");
        for (size_t i = 0; i < build_section->propertyCount; i++)
        {
            const char* value = get_property_value_string(&build_section->properties[i]);
            LOGINFO("build.%s = %s", build_section->properties[i].name, value);
            free((void*)value);
        }

        config_file_close(&cfg);
        return;
    }

    const char* name = args.values[0];
    int index = -1;
    for (size_t i = 0; i < sizeof(config_properties) / sizeof(struct config_property_pair); i++)
    {
        if (strcmp(config_properties[i].name, name) != 0)
            continue;

        index = i;
        break;
    }
    if (index == -1)
    {
        LOGERROR("%s is not a valid config option", name);
        LOGINFO("Valid config options:");
        for (size_t i = 0; i < sizeof(config_properties) / sizeof(struct config_property_pair); i++)
            LOGINFO("\t%s - %s\t%s", config_properties[i].name, config_property_value_type_as_string(config_properties[i].type), config_properties[i].description);
        return;
    }

    config_file_t cfg = {0, 0, 0, 0};
    if (!open_project_file(&cfg))
    {
        LOGERROR("Cannot find project file in current directory");
        return;
    }

    cmd_args_t params = {
        .count=args.count-1,
        .values=args.values+1
    };
    config_properties[index].handle(config_properties[index], params, &cfg);

    save_project_file(&cfg);
    config_file_close(&cfg);
}
