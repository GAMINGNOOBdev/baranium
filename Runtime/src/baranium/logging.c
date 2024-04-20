#include <baranium/logging.h>
#include <baranium/defines.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define LOG_COLOR_COUNT     5
#define LOG_COLOR_NONE      "\033[0m"
#define LOG_COLOR_INFO      "\033[32m"
#define LOG_COLOR_DEBUG     "\033[34m"
#define LOG_COLOR_ERROR     "\033[31m"
#define LOG_COLOR_WARNING   "\033[33m"

static const char* LOG_COLORS[] = {
    LOG_COLOR_NONE,
    LOG_COLOR_INFO,
    LOG_COLOR_DEBUG,
    LOG_COLOR_ERROR,
    LOG_COLOR_WARNING,
};

static const char* loglevelToString(loglevel_t lvl)
{
    switch (lvl)
    {
        default:
            return "[NONE]\t\t";
        
        case LOGLEVEL_INFO:
            return "[INFO]\t\t";

        case LOGLEVEL_DEBUG:
            return "[DEBUG]\t\t";

        case LOGLEVEL_ERROR:
            return "[ERROR]\t\t";

        case LOGLEVEL_WARNING:
            return "[WARNING]\t";
    }

    return "[NONE]\t\t";
}

const char* stringf(const char* formatString, ...)
{
    static char mFormattingBuffer[4096];

    va_list args;
    va_start(args, formatString);
    int result = vsnprintf(mFormattingBuffer, 4096, formatString, args);
    va_end(args);

    return mFormattingBuffer;
}

void logStr(loglevel_t lvl, const char* msg)
{
    if (lvl > LOG_COLOR_COUNT || msg == NULL)
        return;

    #ifndef _WIN32
        printf("%s%s%s%s\n", LOG_COLORS[lvl+1], loglevelToString(lvl), msg, LOG_COLORS[0]);
    #else
        printf("%s%s\n", loglevelToString(lvl), msg);
    #endif
}