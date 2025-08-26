#include "z80.h"
#include <bitset>
#include <iostream>

z80::z80() {
    programCounter = 0;
    reg[A] = 0xFF;
    reg[F] = 0xFF;
    interruptMode = 0;
    interruptVector = 0;
    memoryRefresh = 0;
    IFF1 = 0;
    IFF2 = 0;
    haltState = HALT_NONE;
    eiState = EI_NONE;
}

int z80::cycle() {

    switch(eiState) {

        // Process the next instruction after an EI
        case EI_WAIT:
        {
            eiState = EI_GOOD;
            break;
        }

        // After that instruction set IFF's to actually renable interrupts
        case EI_GOOD:
        {
            eiState = EI_NONE;
            IFF1 = 1;
            IFF2 = 1;
            break;
        }
    }

    std::clog << std::hex << (int)programCounter << ": ";

    int res = 0;
    
    res = process8BitLoadGroup();
    if(res > 0) return res;

    res = process16BitLoadGroup();
    if(res > 0) return res;

    res = processExchangeSearchGroup();
    if(res > 0) return res;

    res = processGeneralArithmeticGroup();
    if(res > 0) return res;

    res = process8BitArithmeticGroup();
    if(res > 0) return res;

    res = process16BitArithmeticGroup();
    if(res > 0) return res;

    res = processRotateShiftGroup();
    if(res > 0) return res;

    res = processBitSetResetTest();
    if(res > 0) return res;

    res = processJumpGroup();
    if(res > 0) return res;

    res = processCallReturnGroup();
    if(res > 0) return res;

    res = processInputOutputGroup();
    if(res > 0) return res;

    return 0;
}

void z80::signalNMI() {

    if(haltState == HALT_WAIT) {
        haltState = HALT_GOOD;
        return;
    }

    // Push pre-interrupt address
    PUSH(programCounter);

    programCounter = 0x0066;

    // Do no process INT until RETN
    IFF2 = IFF1;
    IFF1 = 0;
}

void z80::signalINT() {

    if(haltState == HALT_WAIT) {
        haltState = HALT_GOOD;
        return;
    }

    // Do not capture this interrupt
    if(IFF1 == 0)
        return;

    // Push pre-interrupt address
    PUSH(programCounter);

    // Do no process any further interrupts until EI
    IFF1 = 0;
    IFF2 = 0;

    programCounter = 0x0038;
}

void z80::setFlag(uint8 flag, bool val) {
    reg[F] &= ~flag;

    if(val)
        reg[F] |= flag;
}

bool z80::getFlag(uint8 flag) {
    return reg[F] & flag;
}

void z80::incrementPC(int val) {
    programCounter += val;
    
    // Keep the 7th bit
    uint8 bit = memoryRefresh & 0b10000000;
    uint8 num = memoryRefresh & ~bit;
    num += val;
    num &= ~bit;
    memoryRefresh = bit | num;
}

void z80::ADD(uint8& a, const uint8& b) {
    uint8 res = a + b;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, overflow8(0, a, b));
    setFlag(AddSubtract, 0);
    setFlag(HalfCarry, halfCarry8(a, b));
    setFlag(Carry, carry8(a, b));

    a = res;
}

void z80::ADC(uint8& a, const uint8& b) {
    uint8 c = getFlag(Carry);
    uint8 res = a + b + c;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, overflow8(0, a, b, c));
    setFlag(AddSubtract, 0);
    setFlag(HalfCarry, halfCarry8(a, b, c));
    setFlag(Carry, carry8(a, b, c));

    a = res;
}

void z80::SUB(uint8& a, const uint8& b) {
    uint8 res = a - b;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, overflow8(1, a, b));
    setFlag(AddSubtract, 1);
    setFlag(HalfCarry, halfBorrow8(a, b));
    setFlag(Carry, borrow8(a, b));

    a = res;
}

void z80::SBC(uint8& a, const uint8& b) {
    uint8 c = getFlag(Carry);
    uint8 res = a - b - c;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, overflow8(1, a, b, c));
    setFlag(AddSubtract, 1);
    setFlag(HalfCarry, halfBorrow8(a, b, c));
    setFlag(Carry, borrow8(a, b, c));

    a = res;
}

void z80::AND(uint8& a, const uint8& b) {
    uint8 res = a & b;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, std::bitset<8>(res).count() % 2 == 0);
    setFlag(AddSubtract, 0);
    setFlag(HalfCarry, 1);
    setFlag(Carry, 0);

    a = res;
}

void z80::OR(uint8& a, const uint8& b) {
    uint8 res = a | b;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, std::bitset<8>(res).count() % 2 == 0);
    setFlag(AddSubtract, 0);
    setFlag(HalfCarry, 0);
    setFlag(Carry, 0);

    a = res;
}

void z80::XOR(uint8& a, const uint8& b) {
    uint8 res = a ^ b;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, std::bitset<8>(res).count() % 2 == 0);
    setFlag(AddSubtract, 0);
    setFlag(HalfCarry, 0);
    setFlag(Carry, 0);

    a = res;
}


void z80::CP(const uint8& a, const uint8& b) {
    uint8 res = a - b;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, overflow8(1, a, b));
    setFlag(AddSubtract, 1);
    setFlag(HalfCarry, halfBorrow8(a, b));
    setFlag(Carry, borrow8(a, b));
}

void z80::INC(uint8& a) {
    uint8 res = a + 1;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, a == 0x7F);
    setFlag(AddSubtract, 0);
    setFlag(HalfCarry, halfCarry8(a, 1));

    a = res;
}

void z80::DEC(uint8& a) {
    uint8 res = a - 1;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, a == 0x80);
    setFlag(AddSubtract, 1);
    setFlag(HalfCarry, halfBorrow8(a, 1));

    a = res;
}

void z80::ADD16(uint16& a, const uint16& b) {
    uint16 res = a + b;

    setFlag(HalfCarry, halfCarry16(a, b));
    setFlag(AddSubtract, 0);
    setFlag(Carry, carry16(a, b));

    a = res;
}

void z80::ADC16(uint16& a, const uint16& b) {
    uint16 c = getFlag(Carry);
    uint16 res = a + b + c;

    setFlag(Sign, res & 0b1000000000000000);
    setFlag(Zero, res == 0);
    setFlag(HalfCarry, halfCarry16(a, b, c));
    setFlag(ParityOverflow, overflow16(0, a, b, c));
    setFlag(AddSubtract, 0);
    setFlag(Carry, carry16(a, b, c));

    a = res;
}

void z80::SBC16(uint16& a, const uint16& b) {
    uint8 c = getFlag(Carry);
    uint16 res = a - b - c;

    setFlag(Sign, res & 0b1000000000000000);
    setFlag(Zero, res == 0);
    setFlag(HalfCarry, halfBorrow16(a, b, c));
    setFlag(ParityOverflow, overflow16(1, a, b, c));
    setFlag(AddSubtract, 1);
    setFlag(Carry, borrow16(a, b, c));

    a = res;
}

void z80::INC16(uint16& a) {
    a += 1;
}

void z80::DEC16(uint16& a) {
    a -= 1;
}

void z80::CALL(const uint16& addr) {
    PUSH(programCounter);
    programCounter = addr;
}

void z80::RET() {
    POP(programCounter);
}

void z80::RETN() {
    RET();
    IFF1 = IFF2;
}

void z80::RETI() {
    RET();
}

void z80::RST(const uint8& p) {
    PUSH(programCounter);
    programCounter = p * 8;
}

void z80::BIT(const uint8& bit, const uint8& data) {
    setFlag(Sign, (data & bit & 0b10000000));
    setFlag(ParityOverflow, (data & bit) == 0);
    setFlag(Zero, (data & bit) == 0);
    setFlag(HalfCarry, 1);
    setFlag(AddSubtract, 0);
}

void z80::SET(const uint8& bit, uint8& data) {
    data |= bit;
}

void z80::RES(const uint8& bit, uint8& data) {
    data &= ~bit;
}

void z80::RLC(uint8& num, bool A) {
    setFlag(Carry, num & 0b10000000);

    num = num << 1;
    if(getFlag(Carry)) num |= 0b00000001;

    setFlag(HalfCarry, 0);
    setFlag(AddSubtract, 0);

    if(!A) {
        setFlag(Sign, num & 0b10000000);
        setFlag(Zero, num == 0);
        setFlag(ParityOverflow, std::bitset<8>(num).count() % 2 == 0);
    }
}

void z80::RL(uint8& num, bool A) {
    bool prev = getFlag(Carry);
    setFlag(Carry, num & 0b10000000);

    num = num << 1;
    if(prev) num |= 0b00000001;

    setFlag(HalfCarry, 0);
    setFlag(AddSubtract, 0);

    if(!A) {
        setFlag(Sign, num & 0b10000000);
        setFlag(Zero, num == 0);
        setFlag(ParityOverflow, std::bitset<8>(num).count() % 2 == 0);
    }
}

void z80::RRC(uint8& num, bool A) {
    setFlag(Carry, num & 0b00000001);

    num = num >> 1;
    if(getFlag(Carry)) num |= 0b10000000;

    setFlag(HalfCarry, 0);
    setFlag(AddSubtract, 0);

    if(!A) {
        setFlag(Sign, num & 0b10000000);
        setFlag(Zero, num == 0);
        setFlag(ParityOverflow, std::bitset<8>(num).count() % 2 == 0);
    }
}

void z80::RR(uint8& num, bool A) {
    bool prev = getFlag(Carry);
    setFlag(Carry, num & 0b00000001);

    num = num >> 1;
    if(prev) num |= 0b10000000;

    setFlag(HalfCarry, 0);
    setFlag(AddSubtract, 0);

    if(!A) {
        setFlag(Sign, num & 0b10000000);
        setFlag(Zero, num == 0);
        setFlag(ParityOverflow, std::bitset<8>(num).count() % 2 == 0);
    }
}

void z80::SLA(uint8& num) {
    setFlag(Carry, num & 0b10000000);
    num = num << 1;

    setFlag(Sign, num & 0b10000000);
    setFlag(Zero, num == 0);
    setFlag(HalfCarry, 0);
    setFlag(ParityOverflow, std::bitset<8>(num).count() % 2 == 0);
    setFlag(AddSubtract, 0);
}

void z80::SRA(uint8& num) {
    setFlag(Carry, num & 0b00000001);

    num = num >> 1;
    if(num & 0b01000000)
        num |= 0b10000000;

    setFlag(Sign, num & 0b10000000);
    setFlag(Zero, num == 0);
    setFlag(HalfCarry, 0);
    setFlag(ParityOverflow, std::bitset<8>(num).count() % 2 == 0);
    setFlag(AddSubtract, 0);
}

void z80::SRL(uint8& num) {
    setFlag(Carry, num & 0b00000001);
    num = num >> 1;

    setFlag(Sign, num & 0b10000000);
    setFlag(Zero, num == 0);
    setFlag(HalfCarry, 0);
    setFlag(ParityOverflow, std::bitset<8>(num).count() % 2 == 0);
    setFlag(AddSubtract, 0);
}

void z80::SLL(uint8& num) {
    setFlag(Carry, num & 0b10000000);

    num = num << 1;
    num |= 0b00000001;

    setFlag(Sign, num & 0b10000000);
    setFlag(Zero, num == 0);
    setFlag(HalfCarry, 0);
    setFlag(ParityOverflow, std::bitset<8>(num).count() % 2 == 0);
    setFlag(AddSubtract, 0);
}

void z80::PUSH(const uint16& data) {
    mapper_write(stackPointer - 1, data >> 8);
    mapper_write(stackPointer - 2, data);
    stackPointer -= 2;
}

void z80::POP(uint16& data) {
    data = pairBytes(mapper_read(stackPointer+1), mapper_read(stackPointer));
    stackPointer += 2;
}