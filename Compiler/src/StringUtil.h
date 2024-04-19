#ifndef __STRINGUTIL_H_
#define __STRINGUTIL_H_ 1

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string>
#include <vector>

/**
 * Gets the file extension from a file path
 * @param filePath File path
 * @returns Only the extension from the file path (WITH a `.`)
*/
std::string GetExtension(std::string str);

/**
 * Converts a number to it's hexadecimal representation
 * 
 * @param[in] num integer value
 * 
 * @returns hex value of input number as `std::string`
*/
std::string to_hex_string(size_t num);

/**
 * Checks and counts how many times `c` comes up in `str`
 * 
 * @param[in] str input string
 * @param[in] c desired character
 * 
 * @returns the number of times `c` comes up
*/
uint8_t StrContains(std::string str, char c);

/**
 * Removes all whitespaces from a string
 * 
 * @param[in] str input string
 * 
 * @returns cleared string without whitespaces
*/
std::string StrTrim(std::string str);

/**
 * Removes all whitespaces from the start and end of a string
 * 
 * @param[in] str input string
 * 
 * @returns cleared string without whitespaces at the start and end
*/
std::string StrTrimLeading(std::string str);

/**
 * Removes all "single-lined comments" ( "//" or "#" ) from a string
 * 
 * @param[in] str input string
 * 
 * @returns cleared string without "comments"
*/
std::string StrTrimComment(std::string str);

/**
 * Replaces all uppercase letters with lowercase ones
 * 
 * @param[in] str input string
 * 
 * @returns lowercase string
*/
std::string StrLowercase(std::string str);

/**
 * Replaces all lowercase letters with uppercase ones
 * 
 * @param[in] str input string
 * 
 * @returns uppercase string
*/
std::string StrUppercase(std::string str);

/**
 * Checks if given string is a number
 * 
 * @param[in] str input string
 * 
 * @returns true if it is a number, false otherwise
*/
bool StrIsNumber(std::string str);

/**
 * Checks if given string is a floating point number
 * 
 * @param[in] str input string
 * 
 * @returns true if it is a floating point number, false otherwise
*/
bool StrIsFloat(std::string str);

/**
 * Checks if given string is a bool
 * 
 * @param[in] str input string
 * 
 * @returns true if it is a bool, false otherwise
*/
bool StrIsBool(std::string str);

/**
 * Splits a string using a delimiter
 * 
 * @note This will also remove all delimiters
 * 
 * @param[in] str input string
 * @param[in] delim delimiter
 * 
 * @returns list of each individual parts of the split string
*/
std::vector<std::string> StrSplit(std::string str, char delim);

/**
 * Splits a string using a delimiter WITHOUT removing the delimiters from the end result
 * 
 * @param[in] str input string
 * @param[in] delim delimiter
 * 
 * @returns list of each individual parts of the split string
*/
std::vector<std::string> StrSplitLess(std::string str, char delim);

/**
 * Like printf but for building a string together
 * 
 * @param[in] formatString string which has format information
 * @param[in] ... any other arguments
 * 
 * @returns the new formatted string
*/
const char* stringf(const char* formatString, ...);

#endif