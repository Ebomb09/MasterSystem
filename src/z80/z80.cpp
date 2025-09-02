#include "z80/z80.h"
#include "common/utilities.h"
#include <bitset>
#include <iostream>

z80::z80() {
    programCounter = 0;
    interruptMode = 0;
    interruptVector = 0;
    memoryRefresh = 0;
    IFF1 = 0;
    IFF2 = 0;
    haltState = HALT_NONE;
    eiState = EI_NONE;
}

int z80::cycle() {

    // Process the next instruction after an EI
    if(eiState == EI_WAIT) {
        eiState = EI_GOOD;
    }

    std::clog << std::hex << (int)programCounter << ": ";

    int res = 0;
    
    if(res == 0) res = process8BitLoadGroup();
    if(res == 0) res = process16BitLoadGroup();
    if(res == 0) res = processExchangeSearchGroup();
    if(res == 0) res = processGeneralArithmeticGroup();
    if(res == 0) res = process8BitArithmeticGroup();
    if(res == 0) res = process16BitArithmeticGroup();
    if(res == 0) res = processRotateShiftGroup();
    if(res == 0) res = processBitSetResetTest();
    if(res == 0) res = processJumpGroup();
    if(res == 0) res = processCallReturnGroup();
    if(res == 0) res = processInputOutputGroup();

    // Processed the instruction, reenable interrupts
    if(eiState == EI_GOOD) {
        eiState = EI_NONE;
        IFF1 = 1;
        IFF2 = 1;
    }

    return res;
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

void z80::setFlag(uint8_t flag, bool val) {
    reg[F] &= ~flag;

    if(val)
        reg[F] |= flag;
}

bool z80::getFlag(uint8_t flag) {
    return reg[F] & flag;
}

void z80::incrementPC(int val) {
    programCounter += val;
    
    // Keep the 7th bit
    uint8_t bit = memoryRefresh & 0b10000000;
    uint8_t num = memoryRefresh & ~bit;
    num += val;
    num &= ~bit;
    memoryRefresh = bit | num;
}

void z80::ADD(uint8_t& a, const uint8_t& b) {
    uint8_t res = a + b;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, overflow8(0, a, b));
    setFlag(AddSubtract, 0);
    setFlag(HalfCarry, halfCarry8(a, b));
    setFlag(Carry, carry8(a, b));

    a = res;
}

void z80::ADC(uint8_t& a, const uint8_t& b) {
    uint8_t c = getFlag(Carry);
    uint8_t res = a + b + c;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, overflow8(0, a, b, c));
    setFlag(AddSubtract, 0);
    setFlag(HalfCarry, halfCarry8(a, b, c));
    setFlag(Carry, carry8(a, b, c));

    a = res;
}

void z80::SUB(uint8_t& a, const uint8_t& b) {
    uint8_t res = a - b;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, overflow8(1, a, b));
    setFlag(AddSubtract, 1);
    setFlag(HalfCarry, halfBorrow8(a, b));
    setFlag(Carry, borrow8(a, b));

    a = res;
}

void z80::SBC(uint8_t& a, const uint8_t& b) {
    uint8_t c = getFlag(Carry);
    uint8_t res = a - b - c;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, overflow8(1, a, b, c));
    setFlag(AddSubtract, 1);
    setFlag(HalfCarry, halfBorrow8(a, b, c));
    setFlag(Carry, borrow8(a, b, c));

    a = res;
}

void z80::AND(uint8_t& a, const uint8_t& b) {
    uint8_t res = a & b;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, std::bitset<8>(res).count() % 2 == 0);
    setFlag(AddSubtract, 0);
    setFlag(HalfCarry, 1);
    setFlag(Carry, 0);

    a = res;
}

void z80::OR(uint8_t& a, const uint8_t& b) {
    uint8_t res = a | b;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, std::bitset<8>(res).count() % 2 == 0);
    setFlag(AddSubtract, 0);
    setFlag(HalfCarry, 0);
    setFlag(Carry, 0);

    a = res;
}

void z80::XOR(uint8_t& a, const uint8_t& b) {
    uint8_t res = a ^ b;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, std::bitset<8>(res).count() % 2 == 0);
    setFlag(AddSubtract, 0);
    setFlag(HalfCarry, 0);
    setFlag(Carry, 0);

    a = res;
}


void z80::CP(const uint8_t& a, const uint8_t& b) {
    uint8_t res = a - b;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, overflow8(1, a, b));
    setFlag(AddSubtract, 1);
    setFlag(HalfCarry, halfBorrow8(a, b));
    setFlag(Carry, borrow8(a, b));
}

void z80::INC(uint8_t& a) {
    uint8_t res = a + 1;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, a == 0x7F);
    setFlag(AddSubtract, 0);
    setFlag(HalfCarry, halfCarry8(a, 1));

    a = res;
}

void z80::DEC(uint8_t& a) {
    uint8_t res = a - 1;

    setFlag(Sign, res & 0b10000000);
    setFlag(Zero, res == 0);
    setFlag(ParityOverflow, a == 0x80);
    setFlag(AddSubtract, 1);
    setFlag(HalfCarry, halfBorrow8(a, 1));

    a = res;
}

void z80::ADD16(uint16_t& a, const uint16_t& b) {
    uint16_t res = a + b;

    setFlag(HalfCarry, halfCarry16(a, b));
    setFlag(AddSubtract, 0);
    setFlag(Carry, carry16(a, b));

    a = res;
}

void z80::ADC16(uint16_t& a, const uint16_t& b) {
    uint16_t c = getFlag(Carry);
    uint16_t res = a + b + c;

    setFlag(Sign, res & 0b1000000000000000);
    setFlag(Zero, res == 0);
    setFlag(HalfCarry, halfCarry16(a, b, c));
    setFlag(ParityOverflow, overflow16(0, a, b, c));
    setFlag(AddSubtract, 0);
    setFlag(Carry, carry16(a, b, c));

    a = res;
}

void z80::SBC16(uint16_t& a, const uint16_t& b) {
    uint8_t c = getFlag(Carry);
    uint16_t res = a - b - c;

    setFlag(Sign, res & 0b1000000000000000);
    setFlag(Zero, res == 0);
    setFlag(HalfCarry, halfBorrow16(a, b, c));
    setFlag(ParityOverflow, overflow16(1, a, b, c));
    setFlag(AddSubtract, 1);
    setFlag(Carry, borrow16(a, b, c));

    a = res;
}

void z80::INC16(uint16_t& a) {
    a += 1;
}

void z80::DEC16(uint16_t& a) {
    a -= 1;
}

void z80::CALL(const uint16_t& addr) {
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

void z80::RST(const uint8_t& p) {
    PUSH(programCounter);
    programCounter = p * 8;
}

void z80::BIT(const uint8_t& bit, const uint8_t& data) {
    setFlag(Sign, (data & bit & 0b10000000));
    setFlag(ParityOverflow, (data & bit) == 0);
    setFlag(Zero, (data & bit) == 0);
    setFlag(HalfCarry, 1);
    setFlag(AddSubtract, 0);
}

void z80::SET(const uint8_t& bit, uint8_t& data) {
    data |= bit;
}

void z80::RES(const uint8_t& bit, uint8_t& data) {
    data &= ~bit;
}

void z80::RLC(uint8_t& num, bool A) {
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

void z80::RL(uint8_t& num, bool A) {
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

void z80::RRC(uint8_t& num, bool A) {
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

void z80::RR(uint8_t& num, bool A) {
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

void z80::SLA(uint8_t& num) {
    setFlag(Carry, num & 0b10000000);
    num = num << 1;

    setFlag(Sign, num & 0b10000000);
    setFlag(Zero, num == 0);
    setFlag(HalfCarry, 0);
    setFlag(ParityOverflow, std::bitset<8>(num).count() % 2 == 0);
    setFlag(AddSubtract, 0);
}

void z80::SRA(uint8_t& num) {
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

void z80::SRL(uint8_t& num) {
    setFlag(Carry, num & 0b00000001);
    num = num >> 1;

    setFlag(Sign, num & 0b10000000);
    setFlag(Zero, num == 0);
    setFlag(HalfCarry, 0);
    setFlag(ParityOverflow, std::bitset<8>(num).count() % 2 == 0);
    setFlag(AddSubtract, 0);
}

void z80::SLL(uint8_t& num) {
    setFlag(Carry, num & 0b10000000);

    num = num << 1;
    num |= 0b00000001;

    setFlag(Sign, num & 0b10000000);
    setFlag(Zero, num == 0);
    setFlag(HalfCarry, 0);
    setFlag(ParityOverflow, std::bitset<8>(num).count() % 2 == 0);
    setFlag(AddSubtract, 0);
}

void z80::PUSH(const uint16_t& data) {
    mapper_write(stackPointer - 1, data >> 8);
    mapper_write(stackPointer - 2, data);
    stackPointer -= 2;
}

void z80::POP(uint16_t& data) {
    data = pairBytes(mapper_read(stackPointer+1), mapper_read(stackPointer));
    stackPointer += 2;
}