/**
 * @file Logging.h
 * @author GAMINGNOOBdev (https://github.com/GAMINGNOOBdev)
 * @brief A small utility namespace for logging
 * @version 1.0
 * @date 2024-01-21
 * 
 * @copyright Copyright (c) GAMINGNOOBdev 2024
 * 
 */

#ifndef __LOGGING_H_
#define __LOGGING_H_ 1

#include <string>
#include <vector>

#define LOG_COLOR_NONE      "\033[0m"
#define LOG_COLOR_DEBUG     "\033[34m"
#define LOG_COLOR_WARNING   "\033[33m"
#define LOG_COLOR_ERROR     "\033[31m"
#define LOG_COLOR_INFO      "\033[32m"

/**
 * @brief A namespace that can manage logging output
 */
namespace Logging
{
    /**
     * Logging log severity/level
    */
    enum Level
    {
        Debug,
        Warning,
        Error,
        Info,
    };

    /**
     * @brief If this variable is set to `false` all messages with a level of `Debug` will be discarded
     */
    extern bool DebugMessages;

    /**
     * @brief If this variable is set to `false` output will only be written to a file
     */
    extern bool ConsoleLogging;

    /**
     * @brief If this variable is set to `false` output will not be written to a file
     */
    extern bool FileLogging;

    /**
     * @brief Initializes the logging subsystem (mainly logging to a file)
     * 
     * @param filename Custom filename/filepath to store the logging output
    */
    void Init(const char* filename = nullptr);

    /**
     * @brief Closes the logging subsystem
    */
    void Dispose();

    /**
     * @brief Logs a message with given level
     * 
     * @param[in] message log message
     * @param[in] level logging level/severity
    */
    void Log(const char* message, Level level = Level::Debug);

    /**
     * @brief Logs an error message and exits with given code
     * 
     * @param[in] message log message
     * @param[in] code error code
    */
    void LogErrorExit(const char* message, int code = -1);

}

#endif
