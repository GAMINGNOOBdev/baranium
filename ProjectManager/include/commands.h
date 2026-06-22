#ifndef __COMMANDS_H_
#define __COMMANDS_H_ 1

#include <operations.h>

cmd_args_t cmd_run(cmd_args_t* userparam);
cmd_args_t cmd_build(cmd_args_t* userparam);
cmd_args_t cmd_create(cmd_args_t* userparam);
cmd_args_t cmd_config(cmd_args_t* userparam);
cmd_args_t cmd_defines(cmd_args_t* userparam);
cmd_args_t cmd_help(cmd_args_t* userparam);
cmd_args_t cmd_install(cmd_args_t* userparam);
cmd_args_t cmd_library(cmd_args_t* userparam);

#endif
