#ifndef __COMMANDS_H_
#define __COMMANDS_H_ 1

#include <operations.h>

void cmd_run(cmd_args_t* userparam);
void cmd_build(cmd_args_t* userparam);
void cmd_create(cmd_args_t* userparam);
void cmd_config(cmd_args_t* userparam);
void cmd_defines(cmd_args_t* userparam);
void cmd_help(cmd_args_t* userparam);
void cmd_install(cmd_args_t* userparam);
void cmd_library(cmd_args_t* userparam);

#endif
