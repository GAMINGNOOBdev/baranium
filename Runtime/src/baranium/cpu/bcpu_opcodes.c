#include <baranium/cpu/bcpu_opcodes.h>
#include "instructions.c"
#include <memory.h>
#include <stdio.h>

bcpu_opcode opcodes[MAX_OPCODE_AMOUNT];
bool opcodesInitialized = false;

bcpu_opcode bcpu_opcode_create(const char* name, OPCODE_HANDLE opHandle)
{
    bcpu_opcode opcode;

    // hacky way to assign a const ptr
    *((char**)&opcode.name) = name;
    opcode.handle = opHandle;

    return opcode;
}

void bcpu_opcodes_init()
{
    if (opcodesInitialized)
        return;
    
    opcodesInitialized = true;

    memset(opcodes, 0, sizeof(bcpu_opcode)*MAX_OPCODE_AMOUNT);

    for (int i = 0; i < MAX_OPCODE_AMOUNT; i++)
        opcodes[i] = bcpu_opcode_create("???", INVALID_OPCODE);

    { // 0x00 - 0x0F
        opcodes[0x00] = bcpu_opcode_create("NOP", NOP);
        opcodes[0x01] = bcpu_opcode_create("CCF", CCF);
        opcodes[0x02] = bcpu_opcode_create("SCF", SCF);
        opcodes[0x03] = bcpu_opcode_create("CCV", CCV);
        opcodes[0x04] = bcpu_opcode_create("PUSHCV", PUSHCV);
        opcodes[0x05] = bcpu_opcode_create("POPCV", POPCV);
        opcodes[0x06] = bcpu_opcode_create("PUSHIP", PUSHIP);
        opcodes[0x07] = bcpu_opcode_create("POPIP/RET", POPIP);
        opcodes[0x08] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x09] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x0A] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x0B] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x0C] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x0D] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x0E] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x0F] = bcpu_opcode_create("???", INVALID_OPCODE);
    }

    { // 0x10 - 0x1F
        opcodes[0x10] = bcpu_opcode_create("JMP", JMP);
        opcodes[0x11] = bcpu_opcode_create("JMPOFF", JMPOFF);
        opcodes[0x12] = bcpu_opcode_create("JEQ", JEQ);
        opcodes[0x13] = bcpu_opcode_create("JEQOFF", JEQOFF);
        opcodes[0x14] = bcpu_opcode_create("JNQ", JNQ);
        opcodes[0x15] = bcpu_opcode_create("JNQOFF", JNQOFF);
        opcodes[0x16] = bcpu_opcode_create("JLZ", JLZ);
        opcodes[0x17] = bcpu_opcode_create("JLZOFF", JLZOFF);
        opcodes[0x18] = bcpu_opcode_create("JGZ", JGZ);
        opcodes[0x19] = bcpu_opcode_create("JGZOFF", JGZOFF);
        opcodes[0x1A] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x1B] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x1C] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x1D] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x1E] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x1F] = bcpu_opcode_create("???", INVALID_OPCODE);
    }

    { // 0x20 - 0x2F
        opcodes[0x20] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x21] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x22] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x23] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x24] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x25] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x26] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x27] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x28] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x29] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x2A] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x2B] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x2C] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x2D] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x2E] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x2F] = bcpu_opcode_create("???", INVALID_OPCODE);
    }

    { // 0x30 - 0x3F
        opcodes[0x30] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x31] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x32] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x33] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x34] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x35] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x36] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x37] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x38] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x39] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x3A] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x3B] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x3C] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x3D] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x3E] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x3F] = bcpu_opcode_create("???", INVALID_OPCODE);
    }

    { // 0x40 - 0x4F
        opcodes[0x40] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x41] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x42] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x43] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x44] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x45] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x46] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x47] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x48] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x49] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x4A] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x4B] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x4C] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x4D] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x4E] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x4F] = bcpu_opcode_create("???", INVALID_OPCODE);
    }

    { // 0x50 - 0x5F
        opcodes[0x50] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x51] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x52] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x53] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x54] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x55] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x56] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x57] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x58] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x59] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x5A] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x5B] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x5C] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x5D] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x5E] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x5F] = bcpu_opcode_create("???", INVALID_OPCODE);
    }

    { // 0x60 - 0x6F
        opcodes[0x60] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x61] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x62] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x63] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x64] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x65] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x66] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x67] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x68] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x69] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x6A] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x6B] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x6C] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x6D] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x6E] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x6F] = bcpu_opcode_create("???", INVALID_OPCODE);
    }

    { // 0x70 - 0x7F
        opcodes[0x70] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x71] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x72] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x73] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x74] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x75] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x76] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x77] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x78] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x79] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x7A] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x7B] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x7C] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x7D] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x7E] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x7F] = bcpu_opcode_create("???", INVALID_OPCODE);
    }

    { // 0x80 - 0x8F
        opcodes[0x80] = bcpu_opcode_create("MEM", MEM);
        opcodes[0x81] = bcpu_opcode_create("FEM", FEM);
        opcodes[0x82] = bcpu_opcode_create("SET", SET);
        opcodes[0x83] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x84] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x85] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x86] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x87] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x88] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x89] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x8A] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x8B] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x8C] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x8D] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x8E] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x8F] = bcpu_opcode_create("???", INVALID_OPCODE);
    }

    { // 0x90 - 0x9F
        opcodes[0x90] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x91] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x92] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x93] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x94] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x95] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x96] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x97] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x98] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x99] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x9A] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x9B] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x9C] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x9D] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x9E] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0x9F] = bcpu_opcode_create("???", INVALID_OPCODE);
    }

    { // 0xA0 - 0xAF
        opcodes[0xA0] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xA1] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xA2] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xA3] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xA4] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xA5] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xA6] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xA7] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xA8] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xA9] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xAA] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xAB] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xAC] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xAD] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xAE] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xAF] = bcpu_opcode_create("???", INVALID_OPCODE);
    }

    { // 0xB0 - 0xBF
        opcodes[0xB0] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xB1] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xB2] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xB3] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xB4] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xB5] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xB6] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xB7] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xB8] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xB9] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xBA] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xBB] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xBC] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xBD] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xBE] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xBF] = bcpu_opcode_create("???", INVALID_OPCODE);
    }

    { // 0xC0 - 0xCF
        opcodes[0xC0] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xC1] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xC2] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xC3] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xC4] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xC5] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xC6] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xC7] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xC8] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xC9] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xCA] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xCB] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xCC] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xCD] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xCE] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xCF] = bcpu_opcode_create("???", INVALID_OPCODE);
    }

    { // 0xD0 - 0xDF
        opcodes[0xD0] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xD1] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xD2] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xD3] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xD4] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xD5] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xD6] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xD7] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xD8] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xD9] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xDA] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xDB] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xDC] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xDD] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xDE] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xDF] = bcpu_opcode_create("???", INVALID_OPCODE);
    }

    { // 0xE0 - 0xEF
        opcodes[0xE0] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xE1] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xE2] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xE3] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xE4] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xE5] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xE6] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xE7] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xE8] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xE9] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xEA] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xEB] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xEC] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xED] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xEE] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xEF] = bcpu_opcode_create("???", INVALID_OPCODE);
    }

    { // 0xF0 - 0xFF
        opcodes[0xF0] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xF1] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xF2] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xF3] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xF4] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xF5] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xF6] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xF7] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xF8] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xF9] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xFA] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xFB] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xFC] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xFD] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xFE] = bcpu_opcode_create("???", INVALID_OPCODE);
        opcodes[0xFF] = bcpu_opcode_create("KILL", KILL);
    }
}