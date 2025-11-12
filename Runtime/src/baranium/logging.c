#include <baranium/logging.h>
#include <baranium/defines.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

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

// off by default
static uint8_t logging_debug_messages_enabled = 0;

// on by default
static uint8_t logging_stdout_messages_enabled = 1;

// no output by default
static FILE* logging_log_messages_output_stream = NULL;

static const char* LOG_LEVEL_STRINGS[] = {
    "[ INFO ]  ",
    "[ DEBUG ] ",
    "[ ERROR ] ",
    "[WARNING] "
};

const char* stringf(const char* formatString, ...)
{
    static char mFormattingBuffer[4096];

    va_list args;
    va_start(args, formatString);
    vsnprintf(mFormattingBuffer, 4096, formatString, args);
    va_end(args);

    return mFormattingBuffer;
}

const char* logstringf(const char* fmt, ...)
{
    static char mLoggingFormattingBuffer[4096];

    va_list args;
    va_start(args, fmt);
    vsnprintf(mLoggingFormattingBuffer, 4096, fmt, args);
    va_end(args);

    return mLoggingFormattingBuffer;
}

uint8_t log_enable_debug_msgs(uint8_t val)
{
    if (val != (uint8_t)-1)
        logging_debug_messages_enabled = val;

    return logging_debug_messages_enabled;
}

void log_enable_stdout(uint8_t val)
{
    logging_stdout_messages_enabled = val;
}

void log_set_stream(FILE* stream)
{
    logging_log_messages_output_stream = stream;
}

static char mLastLogMessage[4096];
static char mLastLogMessageRepeats = 0;
void log_msg(loglevel_t lvl, const char* msg, const char* file, int line)
{
    if (lvl >= LOG_COLOR_COUNT - 1 || msg == NULL) return;
    if (!logging_debug_messages_enabled && lvl == LOGLEVEL_DEBUG) return;

    time_t local_time = time(NULL);
    struct tm* tm = localtime(&local_time);

    if (strcmp(mLastLogMessage, msg) == 0)
    {
        
        if (mLastLogMessageRepeats < 4)
            mLastLogMessageRepeats++;
        else if (mLastLogMessageRepeats == 4)
        {
            log_msg(LOGLEVEL_INFO, "Last message repeated 5 times, skipping repeats...", __FILE_NAME__, __LINE__);
            strcpy(mLastLogMessage, msg);
            mLastLogMessageRepeats = 5;
            return;
        }
        else
            return;
    }
    else
    {
        strcpy(mLastLogMessage, msg);
        mLastLogMessageRepeats = 0;
    }

#if BARANIUM_PLATFORM != BARANIUM_PLATFORM_WINDOWS

    if (logging_log_messages_output_stream == stdout)
        fprintf(logging_log_messages_output_stream, "%s[%02d:%02d:%02d] %s(%s:%d): %s%s\n", LOG_COLORS[lvl+1], tm->tm_hour, tm->tm_min, tm->tm_sec, LOG_LEVEL_STRINGS[lvl], file, line, msg, LOG_COLORS[0]);
    else if (logging_log_messages_output_stream != NULL)
        fprintf(logging_log_messages_output_stream, "[%02d:%02d:%02d] %s(%s:%d): %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, LOG_LEVEL_STRINGS[lvl], file, line, msg);

#else

    if (logging_log_messages_output_stream != NULL)
        fprintf(logging_log_messages_output_stream, "[%02d:%02d:%02d] %s(%s:%d): %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, LOG_LEVEL_STRINGS[lvl], file, line, msg);

#endif

    if ((logging_log_messages_output_stream != stdout || logging_log_messages_output_stream != stderr) && logging_log_messages_output_stream != NULL)
        fflush(logging_log_messages_output_stream);

    if (logging_stdout_messages_enabled && logging_log_messages_output_stream != stdout && lvl != LOGLEVEL_DEBUG)
#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
        fprintf(stdout, "[%02d:%02d:%02d] %s(%s:%d): %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, LOG_LEVEL_STRINGS[lvl], file, line, msg);
#else
        fprintf(stdout, "%s[%02d:%02d:%02d] %s(%s:%d): %s%s\n", LOG_COLORS[lvl+1], tm->tm_hour, tm->tm_min, tm->tm_sec, LOG_LEVEL_STRINGS[lvl], file, line, msg, LOG_COLORS[0]);
#endif
}
