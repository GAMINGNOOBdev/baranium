#include "CodeBuilder.h"
#include <memory.h>

namespace Binaries
{

CodeBuilder::CodeBuilder() {}

void CodeBuilder::Clear()
{
    mData.clear();
}

uint8_t* CodeBuilder::Data()
{
    return mData.data();
}

uint64_t CodeBuilder::Size()
{
    return mData.size();
}

bool CodeBuilder::ReturnedFromExecution()
{
    if (mData.size() == 0)
        return true;

    return mData.at(mData.size()-1) == 0x0F;
}

void CodeBuilder::PushStringValue(std::string str)
{
    uint64_t data = 0;

    size_t strLength = str.length();
    size_t leftOverSize = strLength;
    void* value = (void*)str.data();
    void* valPtr = (void*)str.data();
    int index = 0;
    for (index = 0; index * 8 < strLength; index++)
    {
        if (leftOverSize < 8)
            break;

        valPtr = (void*)((uint64_t)value + index*8);
        memcpy(&data, valPtr, 8);
        leftOverSize -= 8;
        PUSH(data);
    }
    valPtr = (void*)((uint64_t)value + index*8);
    data = 0;
    memcpy(&data, valPtr, leftOverSize);
    PUSH(data);
    PUSH(strLength);
    PUSH((uint64_t)Language::VariableType::String);
}

void CodeBuilder::PushBoolValue(bool b)
{
    PUSH(b);
    PUSH(sizeof(bool));
    PUSH((uint64_t)Language::VariableType::Bool);
}

void CodeBuilder::PushUintValue(uint32_t val)
{
    PUSH(val);
    PUSH(sizeof(uint32_t));
    PUSH((uint64_t)Language::VariableType::Uint);
}

void CodeBuilder::PushIntValue(int32_t val)
{
    PUSH(val);
    PUSH(sizeof(int32_t));
    PUSH((uint64_t)Language::VariableType::Int);
}

void CodeBuilder::PushFloatValue(float val)
{
    uint64_t data = 0;
    memcpy(&data, &val, sizeof(float));
    PUSH(data);
    PUSH(sizeof(float));
    PUSH((uint64_t)Language::VariableType::Float);
}

void CodeBuilder::NOP() { push(0x00); }
void CodeBuilder::CCF() { push(0x01); }
void CodeBuilder::SCF() { push(0x02); }
void CodeBuilder::CCV() { push(0x03); }
void CodeBuilder::PUSHCV() { push(0x04); }
void CodeBuilder::POPCV() { push(0x05); }

void CodeBuilder::PUSHVAR(index_t id)
{
    push(0x06);
    push64(id);
}

void CodeBuilder::POPVAR(index_t id)
{
    push(0x07);
    push64(id);
}

void CodeBuilder::PUSH(uint64_t val)
{
    push(0x08);
    push64(val);
}

void CodeBuilder::CALL(index_t id)
{
    push(0x0E);
    push64(id);
}

void CodeBuilder::RET()
{
    push(0x0F);
}

void CodeBuilder::JMP(uint64_t addr)
{
    push(0x10);
    push64(addr);
}

void CodeBuilder::JMPOFF(int16_t offset)
{
    push(0x11);
    push16(offset);
}

void CodeBuilder::JEQ(uint64_t addr)
{
    push(0x12);
    push64(addr);
}

void CodeBuilder::JEQOFF(uint16_t addr)
{
    push(0x13);
    push16(addr);
}

void CodeBuilder::JNQ(uint64_t addr)
{
    push(0x14);
    push64(addr);
}

void CodeBuilder::JNQOFF(uint16_t addr)
{
    push(0x15);
    push16(addr);
}

void CodeBuilder::JLZ(uint64_t addr)
{
    push(0x16);
    push64(addr);
}

void CodeBuilder::JLZOFF(uint16_t addr)
{
    push(0x17);
    push16(addr);
}

void CodeBuilder::JGZ(uint64_t addr)
{
    push(0x18);
    push64(addr);
}

void CodeBuilder::JGZOFF(uint16_t addr)
{
    push(0x19);
    push16(addr);
}

void CodeBuilder::MOD()     { push(0x20); }
void CodeBuilder::DIV()     { push(0x21); }
void CodeBuilder::MUL()     { push(0x22); }
void CodeBuilder::SUB()     { push(0x23); }
void CodeBuilder::ADD()     { push(0x24); }
void CodeBuilder::AND()     { push(0x25); }
void CodeBuilder::OR()      { push(0x26); }
void CodeBuilder::XOR()     { push(0x27); }
void CodeBuilder::SHFTL()   { push(0x28); }
void CodeBuilder::SHFTR()   { push(0x29); }
void CodeBuilder::CMP()     { push(0x30); }

void CodeBuilder::MEM(size_t size, uint8_t type, index_t id)
{
    push(0x80);
    push64(size);
    push(type);
    push64(id);
}

void CodeBuilder::FEM(index_t id)
{
    push(0x81);
    push64(id);
}

void CodeBuilder::SET(index_t id, size_t size, void* data)
{
    push(0x82);
    push64(id);
    push64(size);

    uint8_t* dataPtr = (uint8_t*)data;
    for (int i = 0; i < size; i++)
    {
        push(*dataPtr);
        dataPtr++;
    }
}

void CodeBuilder::INSTANTIATE()     { push(0xD0); }
void CodeBuilder::DELETE()          { push(0xD1); }
void CodeBuilder::ATTACH()          { push(0xD2); }
void CodeBuilder::DETACH()          { push(0xD3); }

void CodeBuilder::KILL(int64_t code)
{
    push(0xFF);
    push64(code);
}

void CodeBuilder::push64(uint64_t data)
{
    mData.push_back((uint8_t)((0xFF00000000000000 & data) >> 56));
    mData.push_back((uint8_t)((0xFF000000000000 & data) >> 48));
    mData.push_back((uint8_t)((0xFF0000000000 & data) >> 40));
    mData.push_back((uint8_t)((0xFF00000000 & data) >> 32));
    mData.push_back((uint8_t)((0xFF000000 & data) >> 24));
    mData.push_back((uint8_t)((0xFF0000 & data) >> 16));
    mData.push_back((uint8_t)((0xFF00 & data) >> 8));
    mData.push_back((uint8_t)(0xFF & data));
}

void CodeBuilder::push32(uint32_t data)
{
    mData.push_back((uint8_t)((0xFF000000 & data) >> 24));
    mData.push_back((uint8_t)((0xFF0000 & data) >> 16));
    mData.push_back((uint8_t)((0xFF00 & data) >> 8));
    mData.push_back((uint8_t)(0xFF & data));
}

void CodeBuilder::push16(uint16_t data)
{
    mData.push_back((uint8_t)((0xFF00 & data) >> 8));
    mData.push_back((uint8_t)(0xFF & data));
}

void CodeBuilder::push(uint8_t data)
{
    mData.push_back(data);
}

}