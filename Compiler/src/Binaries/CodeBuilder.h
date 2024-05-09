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

        // no operation
        void NOP();

        // clear comparison flag
        void CCF();

        // jump if equal to
        void JEQ(uint64_t addr);

        // jump offset-ed from the current position of equal to
        void JEQOFF(uint16_t addr);

        // jump if not equal to
        void JNQ(uint64_t addr);

        // jump offset-ed from the current position of not equal to
        void JNQOFF(uint16_t addr);

        // jump to
        void JMP(uint64_t addr);

        // jump offset-ed from the current position to
        void JMPOFF(int16_t offset);

        // push the compare value to the stack
        void PUSHCV();

        // pop the compare value from the stack
        void POPCV();

        // allocate memory
        void MEM(size_t size, uint64_t id);

        // deallocate memory
        void FEM(uint64_t id);

        // set var data
        void SET(uint64_t id, size_t size, void* data);

    private:
        void push64(uint64_t data);
        void push32(uint32_t data);
        void push16(uint16_t data);
        void push(uint8_t data);

    private:
        std::vector<uint8_t> mData;
    };

}

#endif