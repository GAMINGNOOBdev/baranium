#ifndef __BINARIES__CODEBUILDER_H_
#define __BINARIES__CODEBUILDER_H_ 1

#include "../Language/Types.h"
#include <stdint.h>
#include <vector>

#define CMP_LESS_THAN     0x18
#define CMP_LESS_EQUAL    0x38
#define CMP_GREATER_THAN  0x28
#define CMP_GREATER_EQUAL 0x48
#define CMP_EQUAL         0x08
#define CMP_NOTEQUAL      0xF8

#define CMP_AND 0
#define CMP_OR  1

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

        /**
         * @brief Check if the last instruction is a return statement
         */
        bool ReturnedFromExecution();

        // push a string value to the stack (needed for assignments of strings)
        void PushStringValue(std::string str);

        // push a bool value to the stack
        void PushBoolValue(bool b);

        // push a uint value to the stack
        void PushUintValue(uint32_t val);

        // push a int value to the stack
        void PushIntValue(int32_t val);

        // push a float value to the stack
        void PushFloatValue(float val);

        // no operation
        void NOP();

        // clear compare flag
        void CCF();

        // set compare flag
        void SCF();

        // clear compare value
        void CCV();

        // invert compare value
        void ICV();

        // push the compare value to the stack
        void PUSHCV();

        // pop the compare value from the stack
        void POPCV();

        // push a variable's value to the stack (can't be used on string variables fyi)
        void PUSHVAR(index_t id);

        // pop a value from the stack into a variable
        void POPVAR(index_t id);

        // push a value to the stack
        void PUSH(uint64_t val);

        // call a function with a specific id
        void CALL(index_t id);

        // return from a function
        void RET();

        // jump to
        void JMP(uint64_t addr);

        // jump offset-ed from the current position to
        void JMPOFF(int16_t offset);

        // jump if equal to
        void JMPC(uint64_t addr);

        // jump offset-ed from the current position
        void JMPCOFF(uint16_t addr);

        // modulo two values from the stack
        void MOD();

        // divide two values from the stack
        void DIV();

        // multiply two values from the stack
        void MUL();

        // subtrack two values from the stack
        void SUB();

        // add two values from the stack
        void ADD();

        // bitwise and two values from the stack
        void AND();

        // bitwise or two values from the stack
        void OR();

        // bitwise exclusive-or two values from the stack
        void XOR();

        // bitwise shift left
        void SHFTL();

        // bitwise shift right
        void SHFTR();

        // compare two values on the stack
        void CMP(uint8_t compareMethod);

        // compare two compared values together
        void CMPC(uint8_t compareCombineMethod);

        // allocate memory
        void MEM(size_t size, uint8_t type, index_t id);

        // deallocate memory
        void FEM(index_t id);

        // set var data
        void SET(index_t id, size_t size, void* data);

        // instantiate an object
        void INSTANTIATE();

        // delete an object
        void DELETE();

        // attach to an object
        void ATTACH();

        // detach from an object
        void DETACH();

        // stop execution with return code
        void KILL(int64_t code);

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
