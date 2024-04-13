#ifndef __BARANIUM__LOGGING_H_
#define __BARANIUM__LOGGING_H_ 1

#include "defines.h"

typedef unsigned char loglevel_t;

#define LOGLEVEL_INFO       0
#define LOGLEVEL_DEBUG      1
#define LOGLEVEL_ERROR      2
#define LOGLEVEL_WARNING    3

#define LOG logr
#define LOGINFO(msg) logr(LOGLEVEL_INFO, msg)
#define LOGDEBUG(msg) logr(LOGLEVEL_DEBUG, msg)
#define LOGERROR(msg) logr(LOGLEVEL_ERROR, msg)
#define LOGWARNING(msg) logr(LOGLEVEL_WARNING, msg)

/**
 * Like printf but for building a string together
 * 
 * @param[in] formatString string which has format information
 * @param[in] ... any other arguments
 * 
 * @returns the new formatted string
*/
BARANIUMAPI const char* stringf(const char* formatString, ...);

/**
 * @brief Log a message onto the cmd line
 * 
 * @param lvl Logging level
 * @param msg Log message
 */
BARANIUMAPI void logr(loglevel_t lvl, const char* msg);

#endif