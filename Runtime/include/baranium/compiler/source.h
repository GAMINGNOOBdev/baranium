#ifndef __BARANIUM__COMPILER__SOURCE_H_
#define __BARANIUM__COMPILER__SOURCE_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "source_token.h"
#include <stdio.h>

BARANIUMAPI void baranium_source_open_from_file(baranium_source_token_list* _out, FILE* file);

/**
 * @brief Append tokens of another source into this source
 * 
 * @param other the other source that will be appened
*/
BARANIUMAPI void baranium_source_append_source(baranium_source_token_list* tokens, baranium_source_token_list* other);

/**
 * @brief Parse a single line to source tokens
 * 
 * @param line The string that will be parsed
 * 
 * @returns A list of source tokens
*/
BARANIUMAPI void baranium_source_parse_single_line(baranium_source_token_list* _out, const char* line);

#ifdef __cplusplus
}
#endif

#endif
