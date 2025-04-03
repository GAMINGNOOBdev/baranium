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

#define LOG logStr
#define LOGINFO(msg) logStr(LOGLEVEL_INFO, msg)
#define LOGDEBUG(msg) logStr(LOGLEVEL_DEBUG, msg)
#define LOGERROR(msg) logStr(LOGLEVEL_ERROR, msg)
#define LOGWARNING(msg) logStr(LOGLEVEL_WARNING, msg)

/**
 * @brief Like printf but for building a string together
 * 
 * @param[in] formatString string which has format information
 * @param[in] ... any other arguments
 * 
 * @returns the new formatted string
*/
BARANIUMAPI const char* stringf(const char* formatString, ...);

/**
 * @brief En-/Disable debug messages showing up
 * 
 * @note By default debug messages are off
 * 
 * @param val "Boolean" value, 1 = debug messages show up, 0 = no debug messages, -1 = no change
 *
 * @returns The current log state
 */
BARANIUMAPI uint8_t logEnableDebugMsgs(uint8_t val);

/**
 * @brief En-/Disable messages showing up in stdout
 * 
 * @note By default stdout messages are on
 * 
 * @param val "Boolean" value, 1 = messages show up on stdout, 0 = they do not show up on stdout
 */
BARANIUMAPI void logEnableStdout(uint8_t val);

/**
 * @brief Set an output stream for log messages
 * 
 * @param stream The output stream to which will be written, NULL to disable logging
 */
BARANIUMAPI void logSetStream(FILE* stream);

/**
 * @brief Log a message onto the cmd line
 * 
 * @note Logging it turned off by default, set the output stream first before logging messages
 * 
 * @param lvl Logging level
 * @param msg Log message
 */
BARANIUMAPI void logStr(loglevel_t lvl, const char* msg);

#ifdef __cplusplus
}
#endif

#endif
