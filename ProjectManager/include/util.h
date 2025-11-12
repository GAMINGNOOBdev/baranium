#ifndef __UTIL_H_
#define __UTIL_H_ 1

#include <config.h>

/**
 * @brief Name of the current executable
 */
extern const char* executablename;

char* get_executable_working_directory(void);

/**
 * @brief Get the current working directory
 */
char* get_current_working_directory(void);

/**
 * @brief Open a project file from a specific path
 * 
 * @param file Project file output
 * @param path Path to file
 * @returns 1 on success, 0 on failure
 */
int open_project_file_from(config_file_t* file, const char* path);

/**
 * @brief Open a project file from the current working directory
 * 
 * @param file Project file output
 * @returns 1 on success, 0 on failure
 */
int open_project_file(config_file_t* file);

/**
 * @brief Save a project file to the current working directory
 * 
 * @param file Project file output
 * @returns 1 on success, 0 on failure
 */
int save_project_file(config_file_t* file);

/**
 * @brief Open a project file from a specific path
 * 
 * @param file Project file output
 * @param path Path to file
 * @returns 1 on success, 0 on failure
 */
int copy(const char* input, const char* output);

/**
 * @brief Open a project file from a specific path
 * 
 * @param file Project file output
 * @param path Path to file
 * @returns 1 on success, 0 on failure
 */
const char* get_property_value_string(config_property_t* property);

#endif
