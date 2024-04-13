#include <baranium/bcpu.h>

// converting the flags struct into a readable uint8
uint8_t flags_to_uint8(bcpu_flags flags) {
    uint8_t result;

    result = (flags.C << 0) & (flags.Z << 1) & (flags.I << 2) & (flags.D << 3) & (flags.B << 4) & (flags.UNKNOWN << 5) & (flags.OV << 6) & (flags.N << 7);

    return result;
}

// convers an uint8_t to an bcpu_flags struct
void uint8_to_flags(uint8_t flags, bcpu_flags* flagsPointer) {
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
void _bcpu_fetch(bcpu* _this);

// initializes the cpu with the requered function pointers and also sets the bus of the cpu
void bcpu_init(bcpu* _this)
{
    _this->fetched = 0;
    _this->fetch = _bcpu_fetch;
    _this->ticks = 0;
    bcpu_flags flags;
    _this->flags = flags;
    _this->killTriggered = 0;
}

// this is the method which executes the instructions from the IP value forward
// some instructions can take multiple calls to this function to execute properly
void bcpu_tick(bcpu* _this)
{
    _this->flags.UNKNOWN = 1;

    if (_this->ticks == 0) { // meaning the last instruction is done
        // _this->opcode = _this->bus->read(_this->bus, _this->IP);
        _this->IP++;

        uint8_t addrCycles = 0;// createOpcodeList()[_this->opcode].addrMode(_this); // number of cycles depending on the addess mode
        uint8_t opcodeCycles = 0;// createOpcodeList()[_this->opcode].opcode(_this); // execute instruction and get the number of cycles needed

        uint8_t cycles = addrCycles & opcodeCycles;

        _this->ticks = cycles;//createOpcodeList()[_this->opcode].ticks + cycles;
    }

    _this->ticks--;

    #ifdef _DEBUG
    _this->totalTicks++;
    #endif
}

// resets the cpu
void bcpu_reset(bcpu* _this) {
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
void bcpu_nmi(bcpu* _this) {
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
void bcpu_irq(bcpu* _this) {
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

void _bcpu_fetch(bcpu* _this) {
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