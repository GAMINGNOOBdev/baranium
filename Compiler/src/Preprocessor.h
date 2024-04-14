#ifndef __PREPROCESSOR_H_
#define __PREPROCESSOR_H_ 1

#include "Language/Language.h"
#include <unordered_map>
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

    /**
     * @brief Add a define
     * 
     * @param defineName Define name
     * @param replacement What the define replaces
     */
    static void AddDefine(std::string defineName, std::string replacement);

    /**
     * @brief Check if there are any defines in the currently available tokens and replace if necessary
     * 
     * @param lineTokens Line tokens
     * 
     * @returns A better version of the lines tokens
     */
    static std::vector<SourceToken> AssistInLine(std::vector<SourceToken>& lineTokens);

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
    static std::unordered_map<std::string, std::string> mDefines;
    static std::vector<std::string> mDefinesEntries;
    static std::vector<std::string> mIncludePaths;
};

#endif