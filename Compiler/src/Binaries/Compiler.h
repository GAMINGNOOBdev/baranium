#ifndef __BINARIES__COMPILER_H_
#define __BINARIES__COMPILER_H_ 1

#include "../Language/Types.h"
#include <stdint.h>
#include <vector>

using TokenList = std::vector<std::shared_ptr<Language::Token>>;

namespace Binaries
{

    /**
     * @brief A class that compiles tokens into executable binary code
     */
    struct Compiler
    {
        /**
         * @brief Construct a new `Compiler`
         */
        Compiler();

        /**
         * @brief Clear the currently compiled code
         * 
         * @note This should be called after compiling a function and moving onto the next
         */
        void ClearCompiledCode();

        /**
         * @brief Get the compiled code
         * 
         * @returns A pointer to the compiled code data
         */
        uint8_t* GetCompiledCode();

        /**
         * @brief Get the size in bytes of the compiled code
         * 
         * @returns The size of the compiled code
         */
        size_t GetCompiledCodeSize();

        /**
         * @brief Small utility function of a compiler that copies the value of a variable into a destination buffer
         * 
         * @param __dst The destination buffer that will hold the data
         * @param __src The string representation of the data
         * @param varType The variable type
         */
        void CopyVariableData(void* __dst, std::string __src, Language::VariableType varType);

        /**
         * @brief Compile the given tokens into binary
         * 
         * @param tokens The tokens that will be compiled
         */
        void Compile(TokenList& tokens);

        /**
         * @brief Finalize the compilation of tokens
         */
        void FinalizeCompilation();

    private:
        uint8_t* mCode;
        size_t mCodeLength;
        std::vector<uint8_t> mCodeBuilder;
    };

}

#endif