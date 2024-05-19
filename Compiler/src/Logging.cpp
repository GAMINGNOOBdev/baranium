/**
 * @file Logging.cpp
 * @author GAMINGNOOBdev (https://github.com/GAMINGNOOBdev)
 * @brief A small utility namespace for logging
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

    std::tm* GetCurrentTime()
    {
        std::time_t t = std::time(0);
        return std::localtime(&t);
    }

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

    FILE* LogFile = nullptr;
    bool DebugMessages = true;
    bool ConsoleLogging = true;
    bool FileLogging = true;

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

    void Dispose()
    {
        if (LogFile != nullptr)
            fclose(LogFile);
    }

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

    void LogErrorExit(const char* message, int code)
    {
        Log(message, Error);
        Dispose();
        exit(code);
    }

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