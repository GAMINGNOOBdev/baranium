#include <baranium/file_util.h>
#include <baranium/logging.h>
#include <operations.h>
#include <commands.h>
#include <config.h>
#include <util.h>

void print_help_message(void);
void cmd_help(cmd_args_t* _)
{
    print_help_message();
}
