#include <baranium/file_util.h>
#include <baranium/logging.h>
#include <operations.h>
#include <commands.h>
#include <util.h>

extern void print_help_message(void);

cmd_args_t cmd_help(cmd_args_t* args)
{
    print_help_message();
    return EMPTY_CMD_ARGS;
}
