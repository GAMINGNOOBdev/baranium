#include "CodeBuilder.h"

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

void CodeBuilder::NOP() { mData.push_back(0x00); }

void CodeBuilder::JEQ(uint16_t addr)
{
    mData.push_back(0x15);
    mData.push_back((uint8_t)((0xFF00 & addr) >> 8));
    mData.push_back(0xFF & addr);
}

void CodeBuilder::JMP(uint16_t addr)
{
    mData.push_back(0x13);
    mData.push_back((uint8_t)((0xFF00 & addr) >> 8));
    mData.push_back(0xFF & addr);
}

void CodeBuilder::JMPREL(uint16_t offset)
{
    mData.push_back(0x14);
    mData.push_back((uint8_t)((0xFF00 & offset) >> 8));
    mData.push_back(0xFF & offset);
}

}