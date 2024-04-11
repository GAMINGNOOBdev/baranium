/**
 * @file Logging.cpp
 * @author GAMINGNOOBdev (https://github.com/GAMINGNOOBdev)
 * @brief A small utility class for logging
 * @version 1.0
 * @date 2024-01-21
 * 
 * @copyright Copyright (c) GAMINGNOOBdev 2024
 * 
 */

#include "Logging.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctime>

namespace Logging
{

    /**
     * Gets the current system time
     * 
     * @returns current system time
    */
    std::tm* GetCurrentTime()
    {
        std::time_t t = std::time(0);
        return std::localtime(&t);
    }

    /**
     * Converts a `Level` to it's string counterpart
     * 
     * @param[in] level log level/severity
     * 
     * @returns log level/severity as a string
    */
    const char* GetLogLevelAsString(Level level)
    {
        switch (level)
        {
            case Level::Debug:
                return "[DEBUG]\t";

            case Level::Warning:
                return "[WARNING]";
            
            case Level::Error:
                return "[ERROR]\t";
            
            case Level::Info:
                return "[INFO]\t";
            
            default:
                return "[level]\t";
        }
    }

    #ifndef _WIN32
    /**
     * @brief Get the color string by logging level
     * 
     * @param level The level of logging
     * @return The color modifier string
     */
    const char* GetColorByLevel(Level level)
    {
        switch (level)
        {
            case Level::Debug:
                return LOG_COLOR_DEBUG;

            case Level::Warning:
                return LOG_COLOR_WARNING;

            case Level::Error:
                return LOG_COLOR_ERROR;

            case Level::Info:
                return LOG_COLOR_INFO;
            
            default:
                return LOG_COLOR_NONE;
        }
    }
    #endif

    /**
     * @brief DO NOT TOUCH THIS! DO NOT EDIT THIS MANUALLY!
     * @note DO NOT TOUCH THIS! DO NOT EDIT THIS MANUALLY!
     */
    FILE* LogFile = nullptr;

    /**
     * @brief If this variable is set to `false` all messages with a level of `Debug` will be discarded
     */
    bool DebugMessages = true;

    /**
     * @brief If this variable is set to `false` output will only be written to a file
     */
    bool ConsoleLogging = true;

    /**
     * @brief If this variable is set to `false` output will not be written to a file
     */
    bool FileLogging = true;

    /**
     * @brief Initializes the logging subsystem (mainly logging to a file)
     * 
     * @param filename Custom filename/filepath to store the logging output
    */
    void Init(const char* filename)
    {
        LogFile = nullptr;
        if (!FileLogging)
            return;

        std::tm* now = GetCurrentTime();

        char* logFileName = (char*)filename;
        if (logFileName == nullptr)
            logFileName = (char*)Format("%04d-%02d-%02d_%02d-%02d.%02d.log",
                                        now->tm_year+1900, now->tm_mon+1, now->tm_mday,
                                        now->tm_hour, now->tm_min, now->tm_sec);

        LogFile = fopen(logFileName, "w+");

        const char* logStartLine = Format("Log created at %02d:%02d:%02d on %02d/%02d/%04d\n",
                                        now->tm_hour, now->tm_min, now->tm_sec, now->tm_mon+1, now->tm_mday, now->tm_year+1900);

        if (LogFile == nullptr)
            return;

        fseek(LogFile, 0, SEEK_END);
        fwrite(logStartLine, 1, strlen(logStartLine), LogFile);
        fflush(LogFile);
    }

    /**
     * @brief Closes the logging subsystem
    */
    void Dispose()
    {
        if (LogFile != nullptr)
            fclose(LogFile);
    }

    /**
     * @brief Logs a message with given level
     * 
     * @param[in] message log message
     * @param[in] level logging level/severity
    */
    void Log(const char* message, Level level)
    {
        if (!DebugMessages && level == Level::Debug)
            return;

        std::tm* now = GetCurrentTime();

        std::string messageStr = message;

        std::string logTime = Format("[%02d:%02d:%02d]", now->tm_hour, now->tm_min, now->tm_sec);
        std::string logMessage = Format("%s\t%s\t%s\n", logTime.c_str(), GetLogLevelAsString(level), messageStr.c_str());

        if (LogFile != nullptr && FileLogging)
        {
            fseek(LogFile, 0, SEEK_END);
            fwrite(logMessage.c_str(), 1, logMessage.size(), LogFile);
            fflush(LogFile);
        }

        if (ConsoleLogging)
        {
            #ifdef _WIN32
                printf("%s", logMessage.c_str());
            #else
                printf("%s%s" LOG_COLOR_NONE, GetColorByLevel(level), logMessage.c_str());
            #endif
        }
    }

    /**
     * @brief Logs an error message and exits with given code
     * 
     * @param[in] message log message
     * @param[in] code error code
    */
    void LogErrorExit(const char* message, int code)
    {
        Log(message, Error);
        Dispose();
        exit(code);
    }

    /**
     * @brief Like printf but for building a string together without using `append` on an `std::string` object
     * 
     * @param[in] formatString string which has format information
     * @param[in] ... any other arguments
     * 
     * @returns the new formatted string
    */
    const char* Format(const char* formatString, ...)
    {
        static char mFormattingBuffer[4096];

        va_list args;
        va_start(args, formatString);
        int result = vsnprintf(mFormattingBuffer, 4096, formatString, args);
        va_end(args);

        return mFormattingBuffer;
    }

}