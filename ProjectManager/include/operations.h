#ifndef __OPERATIONS_H_
#define __OPERATIONS_H_ 1

typedef struct cmd_args_t
{
    int count;
    const char** values;
} cmd_args_t;

typedef void(*project_manager_operation_handle_t)(cmd_args_t* userparam);

typedef struct project_manager_operation
{
    const char* name;
    const char* description;
    project_manager_operation_handle_t handle;
} project_manager_operation;

/**
 * @brief Get the corresponding operation according to the user input
 * 
 * @param name Name of the input operation
 * @returns Resulting operation pointer
 */
project_manager_operation* project_manager_operation_get(const char* name);

extern int project_manager_operation_count;
extern project_manager_operation project_manager_operations[];

#endif
