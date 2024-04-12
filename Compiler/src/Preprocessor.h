#ifndef __PREPROCESSOR_H_
#define __PREPROCESSOR_H_ 1

#include "Language/Language.h"
#include <BgeFile.hpp>
#include "Source.h"
#include <string>
#include <vector>

/**
 * @brief A class that handles mostly includes
 */
struct Preprocessor
{
    /**
     * @brief Parse and act upon an operation
     * 
     * @note `operation` should NOT start with a '+'
     * 
     * @param operation The line that describes an operation
     * @param source The source from which the operation came
     */
    static void Parse(std::string operation, Source* source);

    /**
     * @brief Add an include path to search for files
     * 
     * @note there should be something like a config to add/remove include paths, including relative paths
     * 
     * @param path The new include path
     */
    static void AddIncludePath(std::string path);

    /**
     * @brief Remove the last added include path
     */
    static void PopLastInclude();

private:
    /**
     * @brief Search for a file inside the currently given include paths
     * 
     * @param file The file that is required
     * 
     * @returns The full path to the file, empty string if not found
     */
    static std::string SearchIncludePath(std::string file);

private:
    static std::vector<std::string> mIncludePaths;
};

#endif