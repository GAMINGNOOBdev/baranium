#include <baranium/defines.h>
#include <baranium/logging.h>
#include <operations.h>
#include <commands.h>
#include <string.h>
#include <stdio.h>
#include <util.h>

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_APPLE
#   include <stdlib.h>
#else
#   include <malloc.h>
#endif

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
#   include <Windows.h>
#   undef max
#   undef min
#   define OS_DELIMITER '\\'
#else
#   include <sys/stat.h>
#   include <unistd.h>
#   define OS_DELIMITER '/'
#endif

#define PRINT_VERSION printf("Baranium Project Manager Version %d.%d.%d %s\n", BARANIUM_VERSION_YEAR, BARANIUM_VERSION_MONTH, BARANIUM_VERSION_DATE, BARANIUM_VERSION_PHASE)

const char* executablename = NULL;

void print_help_message(void)
{
    printf("barrel [operation] <args>\n");
    printf("Operations:\n");
    for (int i = 0; i < project_manager_operation_count; i++)
    {
        project_manager_operation op = project_manager_operations[i];
        printf("\t%s\t%s\n", op.name, op.description);
    }
    printf("\n");
}

size_t str_index_of(const char* string, char delim)
{
    size_t stringLastSeperatorIndex = -1;
    for (size_t i = 0; string[i] != 0; i++)
        stringLastSeperatorIndex = (string[i] == delim) ? i : stringLastSeperatorIndex;

    return stringLastSeperatorIndex;
}

int main(int argc, const char* argv[])
{
    executablename = get_executable_working_directory();
    if (argc < 2)
    {
        print_help_message();
        return 0;
    }
    cmd_args_t args = (cmd_args_t){
        .count=argc-2,
        .values=&argv[2]
    };

    PRINT_VERSION;
    log_enable_debug_msgs(1);
    log_enable_stdout(1);
    log_set_stream(NULL);

    project_manager_operation* operation = project_manager_operation_get(argv[1]);
    if (operation == NULL)
    {
        print_help_message();
        return -1;
    }

    operation->handle(&args);

    return 0;
}
