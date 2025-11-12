#ifndef __BARANIUM__LOGGING_H_
#define __BARANIUM__LOGGING_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/defines.h>
#include <stdio.h>

typedef unsigned char loglevel_t;

#define LOGLEVEL_INFO       (loglevel_t)0
#define LOGLEVEL_DEBUG      (loglevel_t)1
#define LOGLEVEL_ERROR      (loglevel_t)2
#define LOGLEVEL_WARNING    (loglevel_t)3

#define LOG(level, ...) log_msg(level, logstringf(__VA_ARGS__), __FILE_NAME__, __LINE__)
#define LOGINFO(...) log_msg(LOGLEVEL_INFO, logstringf(__VA_ARGS__), __FILE_NAME__, __LINE__)
#define LOGDEBUG(...) log_msg(LOGLEVEL_DEBUG, logstringf(__VA_ARGS__), __FILE_NAME__, __LINE__)
#define LOGERROR(...) log_msg(LOGLEVEL_ERROR, logstringf(__VA_ARGS__), __FILE_NAME__, __LINE__)
#define LOGWARNING(...) log_msg(LOGLEVEL_WARNING, logstringf(__VA_ARGS__), __FILE_NAME__, __LINE__)

/**
 * @brief Like printf but for building a string together
 * 
 * @param formatString string which has format information
 * @param ... any other arguments
 * 
 * @returns the new formatted string
*/
BARANIUMAPI const char* stringf(const char* formatString, ...);

/**
 * @brief Like printf but for building a string together
 * 
 * @param fmt string which has format information
 * @param ... any other arguments
 * 
 * @returns the new formatted string
*/
BARANIUMAPI const char* logstringf(const char* fmt, ...);

/**
 * @brief En-/Disable debug messages showing up
 * 
 * @note By default debug messages are off
 * 
 * @param val "Boolean" value, 1 = debug messages show up, 0 = no debug messages, -1 = no change
 *
 * @returns The current log state
 */
BARANIUMAPI uint8_t log_enable_debug_msgs(uint8_t val);

/**
 * @brief En-/Disable messages showing up in stdout
 * 
 * @note By default stdout messages are on
 * 
 * @param val "Boolean" value, 1 = messages show up on stdout, 0 = they do not show up on stdout
 */
BARANIUMAPI void log_enable_stdout(uint8_t val);

/**
 * @brief Set an output stream for log messages
 * 
 * @param stream The output stream to which will be written, NULL to disable logging
 */
BARANIUMAPI void log_set_stream(FILE* stream);

/**
 * @brief Log a message onto the cmd line
 * 
 * @note Logging it turned off by default, set the output stream first before logging messages
 * 
 * @param lvl Logging level
 * @param msg Log message
 * @param file File
 * @param line Line
 */
BARANIUMAPI void log_msg(loglevel_t lvl, const char* msg, const char* file, int line);

#ifdef __cplusplus
}
#endif

#endif
