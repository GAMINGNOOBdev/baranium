#include <baranium/cpu/bcpu_opcodes.h>
#include "instructions.c"
#include <memory.h>
#include <stdio.h>

bcpu_opcode opcodes[MAX_OPCODE_AMOUNT];
bool opcodesInitialized = false;

bcpu_opcode createOpcode(const char* name, OPCODE_HANDLE opHandle)
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
        opcodes[i] = createOpcode("???", INVALID_OPCODE);

    { // 0x00 - 0x0F
        opcodes[0x00] = createOpcode("NOP", NOP);
        opcodes[0x01] = createOpcode("CCF", CCF);
        opcodes[0x02] = createOpcode("SCF", SCF);
        opcodes[0x03] = createOpcode("CCV", CCV);
        opcodes[0x04] = createOpcode("PUSHCV", PUSHCV);
        opcodes[0x05] = createOpcode("POPCV", POPCV);
        opcodes[0x06] = createOpcode("PUSHVAR", PUSHVAR);
        opcodes[0x07] = createOpcode("POPVAR", POPVAR);
        opcodes[0x08] = createOpcode("PUSH", PUSH);
        opcodes[0x09] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x0A] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x0B] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x0C] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x0D] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x0E] = createOpcode("PUSHIP", PUSHIP);
        opcodes[0x0F] = createOpcode("POPIP/RET", POPIP);
    }

    { // 0x10 - 0x1F
        opcodes[0x10] = createOpcode("JMP", JMP);
        opcodes[0x11] = createOpcode("JMPOFF", JMPOFF);
        opcodes[0x12] = createOpcode("JEQ", JEQ);
        opcodes[0x13] = createOpcode("JEQOFF", JEQOFF);
        opcodes[0x14] = createOpcode("JNQ", JNQ);
        opcodes[0x15] = createOpcode("JNQOFF", JNQOFF);
        opcodes[0x16] = createOpcode("JLZ", JLZ);
        opcodes[0x17] = createOpcode("JLZOFF", JLZOFF);
        opcodes[0x18] = createOpcode("JGZ", JGZ);
        opcodes[0x19] = createOpcode("JGZOFF", JGZOFF);
        opcodes[0x1A] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x1B] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x1C] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x1D] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x1E] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x1F] = createOpcode("???", INVALID_OPCODE);
    }

    { // 0x20 - 0x2F
        opcodes[0x20] = createOpcode("MOD", MOD);
        opcodes[0x21] = createOpcode("DIV", DIV);
        opcodes[0x22] = createOpcode("MUL", MUL);
        opcodes[0x23] = createOpcode("SUB", SUB);
        opcodes[0x24] = createOpcode("ADD", ADD);
        opcodes[0x25] = createOpcode("AND", AND);
        opcodes[0x26] = createOpcode("OR", OR);
        opcodes[0x27] = createOpcode("XOR", XOR);
        opcodes[0x28] = createOpcode("SHFTL", SHFTL);
        opcodes[0x29] = createOpcode("SHFTR", SHFTR);
        opcodes[0x2A] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x2B] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x2C] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x2D] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x2E] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x2F] = createOpcode("???", INVALID_OPCODE);
    }

    { // 0x30 - 0x3F
        opcodes[0x30] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x31] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x32] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x33] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x34] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x35] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x36] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x37] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x38] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x39] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x3A] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x3B] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x3C] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x3D] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x3E] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x3F] = createOpcode("???", INVALID_OPCODE);
    }

    { // 0x40 - 0x4F
        opcodes[0x40] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x41] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x42] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x43] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x44] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x45] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x46] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x47] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x48] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x49] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x4A] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x4B] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x4C] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x4D] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x4E] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x4F] = createOpcode("???", INVALID_OPCODE);
    }

    { // 0x50 - 0x5F
        opcodes[0x50] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x51] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x52] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x53] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x54] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x55] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x56] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x57] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x58] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x59] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x5A] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x5B] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x5C] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x5D] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x5E] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x5F] = createOpcode("???", INVALID_OPCODE);
    }

    { // 0x60 - 0x6F
        opcodes[0x60] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x61] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x62] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x63] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x64] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x65] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x66] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x67] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x68] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x69] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x6A] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x6B] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x6C] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x6D] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x6E] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x6F] = createOpcode("???", INVALID_OPCODE);
    }

    { // 0x70 - 0x7F
        opcodes[0x70] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x71] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x72] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x73] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x74] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x75] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x76] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x77] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x78] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x79] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x7A] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x7B] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x7C] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x7D] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x7E] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x7F] = createOpcode("???", INVALID_OPCODE);
    }

    { // 0x80 - 0x8F
        opcodes[0x80] = createOpcode("MEM", MEM);
        opcodes[0x81] = createOpcode("FEM", FEM);
        opcodes[0x82] = createOpcode("SET", SET);
        opcodes[0x83] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x84] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x85] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x86] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x87] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x88] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x89] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x8A] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x8B] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x8C] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x8D] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x8E] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x8F] = createOpcode("???", INVALID_OPCODE);
    }

    { // 0x90 - 0x9F
        opcodes[0x90] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x91] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x92] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x93] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x94] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x95] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x96] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x97] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x98] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x99] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x9A] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x9B] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x9C] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x9D] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x9E] = createOpcode("???", INVALID_OPCODE);
        opcodes[0x9F] = createOpcode("???", INVALID_OPCODE);
    }

    { // 0xA0 - 0xAF
        opcodes[0xA0] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xA1] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xA2] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xA3] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xA4] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xA5] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xA6] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xA7] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xA8] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xA9] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xAA] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xAB] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xAC] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xAD] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xAE] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xAF] = createOpcode("???", INVALID_OPCODE);
    }

    { // 0xB0 - 0xBF
        opcodes[0xB0] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xB1] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xB2] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xB3] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xB4] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xB5] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xB6] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xB7] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xB8] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xB9] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xBA] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xBB] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xBC] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xBD] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xBE] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xBF] = createOpcode("???", INVALID_OPCODE);
    }

    { // 0xC0 - 0xCF
        opcodes[0xC0] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xC1] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xC2] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xC3] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xC4] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xC5] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xC6] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xC7] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xC8] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xC9] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xCA] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xCB] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xCC] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xCD] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xCE] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xCF] = createOpcode("???", INVALID_OPCODE);
    }

    { // 0xD0 - 0xDF
        opcodes[0xD0] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xD1] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xD2] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xD3] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xD4] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xD5] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xD6] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xD7] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xD8] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xD9] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xDA] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xDB] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xDC] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xDD] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xDE] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xDF] = createOpcode("???", INVALID_OPCODE);
    }

    { // 0xE0 - 0xEF
        opcodes[0xE0] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xE1] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xE2] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xE3] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xE4] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xE5] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xE6] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xE7] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xE8] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xE9] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xEA] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xEB] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xEC] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xED] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xEE] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xEF] = createOpcode("???", INVALID_OPCODE);
    }

    { // 0xF0 - 0xFF
        opcodes[0xF0] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xF1] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xF2] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xF3] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xF4] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xF5] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xF6] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xF7] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xF8] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xF9] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xFA] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xFB] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xFC] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xFD] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xFE] = createOpcode("???", INVALID_OPCODE);
        opcodes[0xFF] = createOpcode("KILL", KILL);
    }
}