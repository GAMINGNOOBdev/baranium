#ifndef __BARANIUM__FILE_UTIL_H_
#define __BARANIUM__FILE_UTIL_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/string_util.h>
#include <stdint.h>
#include <stddef.h>

#define BARANIUM_FILE_UTIL_FILTER_MASK_FILES                      0x00 // Only files inside the given directory
#define BARANIUM_FILE_UTIL_FILTER_MASK_ALL_FILES                  0x01 // All files including files from subfolders
#define BARANIUM_FILE_UTIL_FILTER_MASK_FOLDERS                    0x02 // Only folder inside the given directory
#define BARANIUM_FILE_UTIL_FILTER_MASK_ALL_FOLDERS                0x03 // All folders including folders from subdirectories
#define BARANIUM_FILE_UTIL_FILTER_MASK_FILES_AND_FOLDERS          0x04 // Only files and folders inside the given directory
#define BARANIUM_FILE_UTIL_FILTER_MASK_ALL_FILES_AND_FOLDERS      0x05 // All files and folders including those in subdirectories

typedef void (*baranium_file_util_iteration_callback_t)(const char* filename);

/**
 * @brief Iterate over the contents of the given directory
 * 
 * @note This function may take a while to complete since it will retrieve all files from subfolders as well if requested
 * 
 * @param path Path to the directory
 * @param mask A filter which decides how a directories' contents shall be gotten
 * @param callback Function which will be called on each iteration/entry
*/
BARANIUMAPI void baranium_file_util_iterate_directory(const char* path, int mask, baranium_file_util_iteration_callback_t callback);

/**
 * @brief Check if given path is a directory and exists
 * 
 * @param path Path to the directory
 * 
 * @returns 0 if no, 1 if yes
 */
BARANIUMAPI uint8_t baranium_file_util_directory_exists(const char* path);

/**
 * @brief Create a directory
 * 
 * @param path Path to the directory
 */
BARANIUMAPI void baranium_file_util_create_directory(const char* path);

/**
 * @brief Gets the contents of the given directory
 * 
 * @note This function may take a while to complete since it will retrieve all files from subfolders as well if specified
 * 
 * @param out Output string list
 * @param path Path to the directory
 * @param mask A filter which decides how a directories' contents shall be gotten
 * 
 * @returns A list of files inside the given directory
*/
BARANIUMAPI baranium_string_list baranium_file_util_get_directory_contents(const char* path, int mask);

#ifdef __cplusplus
}
#endif

#endif
