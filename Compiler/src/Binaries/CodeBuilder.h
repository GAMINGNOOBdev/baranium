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

        // clear compare flag
        void CCF();

        // set compare flag
        void SCF();

        // clear compare value
        void CCV();

        // push the compare value to the stack
        void PUSHCV();

        // pop the compare value from the stack
        void POPCV();

        // jump to
        void JMP(uint64_t addr);

        // jump offset-ed from the current position to
        void JMPOFF(int16_t offset);

        // jump if equal to
        void JEQ(uint64_t addr);

        // jump offset-ed from the current position
        void JEQOFF(uint16_t addr);

        // jump if not equal to
        void JNQ(uint64_t addr);

        // jump offset-ed from the current position to
        void JNQOFF(uint16_t addr);

        // jump if less than zero to
        void JLZ(uint64_t addr);

        // jump offset-ed from the current position to
        void JLZOFF(uint16_t addr);

        // jump if greater than zero to
        void JGZ(uint64_t addr);

        // jump offset-ed from the current position to
        void JGZOFF(uint16_t addr);

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