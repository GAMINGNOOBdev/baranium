#ifndef __PREPROCESSOR_H_
#define __PREPROCESSOR_H_ 1

#include "Language/Language.h"
#include <BgeFile.hpp>
#include "Source.h"
#include <string>
#include <vector>

/**
 * @brief A class that handles mostly includes
 */
struct Preprocessor
{
    /**
     * @brief Parse and act upon an operation
     * 
     * @note `operation` should NOT start with a '+'
     * 
     * @param operation The line that describes an operation
     * @param source The source from which the operation came
     */
    static void Parse(std::string operation, Source* source);
};

#endif