#include <operations.h>
#include <commands.h>
#include <string.h>

int project_manager_operation_count = 8;
project_manager_operation project_manager_operations[] =
{
    {.name = "run", .description = "Runs the built project, add additional parameters to pass command line arguments", .handle = cmd_run},
    {.name = "build", .description = "Builds the project, no params needed.", .handle = cmd_build},
    {.name = "create", .description = "Create a project in the current directory or with a name if given", .handle = cmd_create},
    {.name = "config", .description = "Configure project values", .handle = cmd_config},
    {.name = "defines", .description = "Configure defines and their values", .handle = cmd_defines},
    {.name = "install", .description = "Install the current project (wont do anything if its an executable)", .handle = cmd_install},
    {.name = "library", .description = "Configure libraries", .handle = cmd_library},
    {.name = "help", .description = "Show help dialogue", .handle = cmd_help},
};

project_manager_operation* project_manager_operation_get(const char* name)
{
    for (int i = 0; i < project_manager_operation_count; i++)
    {
        if (strcmp(project_manager_operations[i].name, name) != 0)
            continue;

        return &project_manager_operations[i];
    }

    return NULL;
}
