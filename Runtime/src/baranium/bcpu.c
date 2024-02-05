#include <baranium/bcpu.h>

// converting the flags struct into a readable uint8
uint8_t flags_to_uint8(BCpu_flags flags) {
    uint8_t result;

    result = (flags.C << 0) & (flags.Z << 1) & (flags.I << 2) & (flags.D << 3) & (flags.B << 4) & (flags.UNKNOWN << 5) & (flags.OV << 6) & (flags.N << 7);

    return result;
}

// convers an uint8_t to an BCpu_flags struct
void uint8_to_flags(uint8_t flags, BCpu_flags* flagsPointer) {
    flagsPointer->C = flags & (1 << 0);
    flagsPointer->Z = flags & (1 << 1);
    flagsPointer->I = flags & (1 << 2);
    flagsPointer->D = flags & (1 << 3);
    flagsPointer->B = flags & (1 << 4);
    flagsPointer->UNKNOWN = 1;
    flagsPointer->OV = flags & (1 << 6);
    flagsPointer->N = flags & (1 << 7);
}

// function references (all declared at the end of this file)
void _BCpu_fetch(BCpu* _this);

// initializes the cpu with the requered function pointers and also sets the bus of the cpu
void BCpu_init(BCpu* _this)
{
    _this->fetched = 0;
    _this->fetch = _BCpu_fetch;
    _this->ticks = 0;
    BCpu_flags flags;
    _this->flags = flags;
    _this->killTriggered = 0;
}

// this is the method which executes the instructions from the IP value forward
// some instructions can take multiple calls to this function to execute properly
void BCpu_tick(BCpu* _this)
{
    _this->flags.UNKNOWN = 1;

    if (_this->ticks == 0) { // meaning the last instruction is done
        // _this->opcode = _this->bus->read(_this->bus, _this->IP);
        _this->IP++;
        
        uint8_t addrCycles = 0;// createOpcodeList()[_this->opcode].addrMode(_this); // number of cycles depending on the addess mode
        uint8_t opcodeCycles = 0;// createOpcodeList()[_this->opcode].opcode(_this); // execute instruction and get the number of cycles needed

        uint8_t cycles = addrCycles & opcodeCycles;

        _this->ticks = cycles;//createOpcodeList()[_this->opcode].ticks + cycles;

        /*
        #ifdef _DEBUG
        char* addrModeString = "none";

        if (createOpcodeList()[_this->opcode].addrMode == IMPLICIT)
            addrModeString="IMPLICIT";
        if (createOpcodeList()[_this->opcode].addrMode == IMMEDIATE)
            addrModeString="IMMEDIATE";
        if (createOpcodeList()[_this->opcode].addrMode == ZEROPAGE)
            addrModeString="ZEROPAGE";
        if (createOpcodeList()[_this->opcode].addrMode == ZEROPAGEx)
            addrModeString="ZEROPAGEx";
        if (createOpcodeList()[_this->opcode].addrMode == ZEROPAGEy)
            addrModeString="ZEROPAGEy";
        if (createOpcodeList()[_this->opcode].addrMode == RELATIVE)
            addrModeString="RELATIVE";
        if (createOpcodeList()[_this->opcode].addrMode == ABSOLUTE)
            addrModeString="";
        if (createOpcodeList()[_this->opcode].addrMode == ABSOLUTEx)
            addrModeString="ABSOLUTEx";
        if (createOpcodeList()[_this->opcode].addrMode == ABSOLUTEy)
            addrModeString="ABSOLUTEy";
        if (createOpcodeList()[_this->opcode].addrMode == INDIRECT)
            addrModeString="INDIRECT";
        if (createOpcodeList()[_this->opcode].addrMode == INDIRECTx)
            addrModeString="INDIRECTx";
        if (createOpcodeList()[_this->opcode].addrMode == INDIRECTy)
            addrModeString="INDIRECTy";

        printf("%s\t", addrModeString);

        printf("IP: 0x%x, Cycles: 0x%x, Ticks (total): 0x%x\nTicks (opcode): 0x%x, Opcode: 0x%x, Instruction: %s\n",
               _this->IP, cycles, _this->ticks, createOpcodeList()[_this->opcode].ticks, 
               _this->opcode, createOpcodeList()[_this->opcode].name);
        //#endif
        printf("A: 0x%x, X: 0x%x, Y: 0x%x, STACK: 0x%x, FLAGS: 0x%x, FETCHED: 0x%x, ADDR_ABS: 0x%x, ADDR_REL: 0x%x\n\n",
               _this->A, _this->X, _this->Y,
               _this->stack, flags_to_uint8(_this->flags),
               _this->fetched, _this->ADDR_ABS, _this->ADDR_REL);
        #endif
        */
    }

    _this->ticks--;

    #ifdef _DEBUG
    _this->totalTicks++;
    #endif
}

// resets the cpu
void BCpu_reset(BCpu* _this) {
    _this->ADDR_ABS = 0xFFFC;
    
    uint16_t low = 0;//_this->bus->read(_this->bus, _this->ADDR_ABS);
    uint16_t high = 0;//_this->bus->read(_this->bus, _this->ADDR_ABS+1);

    _this->IP = low | (high << 8);

    _this->A = 0;
    _this->X = 0;
    _this->Y = 0;

    _this->stack = 0xFD;
    uint8_to_flags((1 << 5), &_this->flags);

    _this->ADDR_ABS=0;
    _this->ADDR_REL=0;
    _this->fetched=0;

    _this->ticks = 8;
}

// non-maskable interrupt
void BCpu_nmi(BCpu* _this) {
    // _this->bus->write(_this->bus, 0x100 + _this->stack, (_this->IP >> 8) & 0x00FF);
    _this->stack--;

    // _this->bus->write(_this->bus, 0x100 + _this->stack, _this->IP & 0x00FF);
    _this->stack--;

    _this->flags.B = 0;
    _this->flags.UNKNOWN = 1;
    _this->flags.I = 1;

    // _this->bus->write(_this->bus, 0x100 + _this->stack, flags_to_uint8(_this->flags));
    _this->stack--;

    _this->ADDR_ABS = 0xFFFA;

    uint16_t low = 0;// _this->bus->read(_this->bus, _this->ADDR_ABS);
    uint16_t high = 0;// _this->bus->read(_this->bus, _this->ADDR_ABS + 1);

    _this->IP = (high << 8) | low;

    _this->ticks = 8;
}

// interrupt request
void BCpu_irq(BCpu* _this) {
    if (_this->flags.I) {
        // _this->bus->write(_this->bus, 0x100 + _this->stack, (_this->IP >> 8) & 0x00FF);
        _this->stack--;

        // _this->bus->write(_this->bus, 0x100 + _this->stack, _this->IP & 0x00FF);
        _this->stack--;

        _this->flags.B = 0;
        _this->flags.UNKNOWN = 1;
        _this->flags.I = 1;

        // _this->bus->write(_this->bus, 0x100 + _this->stack, flags_to_uint8(_this->flags));
        _this->stack--;

        _this->ADDR_ABS = 0xFFFE;

        uint16_t low = 0;// _this->bus->read(_this->bus, _this->ADDR_ABS);
        uint16_t high = 0;// _this->bus->read(_this->bus, _this->ADDR_ABS + 1);

        _this->IP = (high << 8) | low;

        _this->ticks = 7;
    }
}


// function declarations

void _BCpu_fetch(BCpu* _this) {
    /*
    if (createOpcodeList()[_this->opcode].addrMode != IMPLICIT)
    {
        _this->fetched = _this->bus->read(_this->bus, _this->ADDR_ABS);
        #ifdef _DEBUG
        printf("FETCHED 0x%x from 0x%x\n", _this->fetched, _this->ADDR_ABS);
        #endif
    }
    */
}