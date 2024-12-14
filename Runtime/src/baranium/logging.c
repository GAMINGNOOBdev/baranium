#include <baranium/logging.h>
#include <baranium/defines.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>

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

// no output by default
static FILE* logging_log_messages_output_stream = NULL;

static const char* LOG_LEVEL_STRINGS[] = {
    "[INFO]\t\t",
    "[DEBUG]\t\t",
    "[ERROR]\t\t",
    "[WARNING]\t"
};

const char* stringf(const char* formatString, ...)
{
    static char mFormattingBuffer[4096];

    va_list args;
    va_start(args, formatString);
    int result = vsnprintf(mFormattingBuffer, 4096, formatString, args);
    va_end(args);

    return mFormattingBuffer;
}

void logEnableDebugMsgs(uint8_t val)
{
    logging_debug_messages_enabled = val;
}

void logSetStream(FILE* stream)
{
    logging_log_messages_output_stream = stream;
}

void logStr(loglevel_t lvl, const char* msg)
{
    if (lvl >= LOG_COLOR_COUNT - 1 || msg == NULL) return;
    if (logging_log_messages_output_stream == NULL) return;
    if (!logging_debug_messages_enabled && lvl == LOGLEVEL_DEBUG) return;

#if BARANIUM_PLATFORM != BARANIUM_PLATFORM_WINDOWS

    if (logging_log_messages_output_stream == stdout)
        fprintf(logging_log_messages_output_stream, "%s%s%s%s\n", LOG_COLORS[lvl+1], LOG_LEVEL_STRINGS[lvl], msg, LOG_COLORS[0]);
    else
        fprintf(logging_log_messages_output_stream, "%s%s\n", LOG_LEVEL_STRINGS[lvl], msg);

#else

    fprintf(logging_log_messages_output_stream, "%s%s\n", LOG_LEVEL_STRINGS[lvl], msg);

#endif

    if (logging_log_messages_output_stream != stdout || logging_log_messages_output_stream != stderr)
        fflush(logging_log_messages_output_stream);
}
