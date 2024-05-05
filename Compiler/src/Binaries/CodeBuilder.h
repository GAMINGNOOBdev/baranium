#ifndef __BINARIES__CODEBUILDER_H_
#define __BINARIES__CODEBUILDER_H_ 1

#include "../Language/Types.h"
#include <stdint.h>
#include <vector>

namespace Binaries
{

    /**
     * @brief A class that builds the code using functions for each instruction
     */
    struct CodeBuilder
    {
        /**
         * @brief Construct a new `CodeBuilder` object
         */
        CodeBuilder();

        /**
         * @brief Clear the currently held code
         */
        void Clear();

        /**
         * @brief Get the current code data
         */
        uint8_t* Data();

        /**
         * @brief Get the current code size
         */
        uint64_t Size();

        void NOP();
        void JEQ(uint16_t addr);
        void JMP(uint16_t addr);
        void JMPREL(uint16_t offset);

    private:
        std::vector<uint8_t> mData;
    };

}

#endif