#include "z80/z80.h"
#include "common/utilities.h"
#include <stdexcept>

uint8_t z80::read_rrrSymbol(uint8_t rrr) {

    // Get the 3bit constrained conversion of r-symbol to register
    rrr &= 0b111;

    switch(rrr) {
        case 0: return reg[B];
        case 1: return reg[C];
        case 2: return reg[D];
        case 3: return reg[E];
        case 4: return reg[H];
        case 5: return reg[L];
        case 6: return reg[F];
        case 7: return reg[A];
    }

    throw(std::invalid_argument("Invalid rrr-symbol"));
}

void z80::write_rrrSymbol(uint8_t rrr, uint8_t data) {
    rrr &= 0b111;

    switch(rrr) {
        case 0: reg[B] = data; return;
        case 1: reg[C] = data; return;
        case 2: reg[D] = data; return;
        case 3: reg[E] = data; return;
        case 4: reg[H] = data; return;
        case 5: reg[L] = data; return;
        case 6: reg[F] = data; return;
        case 7: reg[A] = data; return;
    }
    throw(std::invalid_argument("Invalid rrr-symbol"));
}

const char* z80::name_rrrSymbol(uint8_t rrr) {
    rrr &= 0b111;

    switch(rrr) {
        case 0: return "B";
        case 1: return "C";
        case 2: return "D";
        case 3: return "E";
        case 4: return "H";
        case 5: return "L";
        case 6: return "F";
        case 7: return "A";
    }
    return "";
}

uint16_t z80::read_ddSymbol(uint8_t dd) {

    // Get the 2bit constrained conversion of r-symbol to register
    dd &= 0b11;

    switch(dd) {
        case 0: return pairBytes(reg[B], reg[C]);
        case 1: return pairBytes(reg[D], reg[E]);
        case 2: return pairBytes(reg[H], reg[L]);
        case 3: return stackPointer;
    }
    throw(std::invalid_argument("Invalid dd-symbol"));
}

void z80::write_ddSymbol(uint8_t dd, uint16_t data) {
    dd &= 0b11;

    switch(dd) {
        case 0: reg[B] = (data >> 8); reg[C] = data; return;
        case 1: reg[D] = (data >> 8); reg[E] = data; return;
        case 2: reg[H] = (data >> 8); reg[L] = data; return;
        case 3: stackPointer = data; return;
    }
    throw(std::invalid_argument("Invalid dd-symbol"));
}

const char* z80::name_ddSymbol(uint8_t dd) {
    dd &= 0b11;

    switch(dd) {
        case 0: return "BC";
        case 1: return "DE";
        case 2: return "HL";
        case 3: return "SP";
    }
    return "";
}

uint16_t z80::read_ssSymbol(uint8_t ss) {
    return read_ddSymbol(ss);
}

void z80::write_ssSymbol(uint8_t ss, uint16_t data) {
    write_ddSymbol(ss, data);
}

const char* z80::name_ssSymbol(uint8_t ss) {
    return name_ddSymbol(ss);
}

uint16_t z80::read_qqSymbol(uint8_t qq) {

    // Get the 2bit constrained conversion of q-symbol to register
    qq &= 0b11;

    switch(qq) {
        case 0: return pairBytes(reg[B], reg[C]);
        case 1: return pairBytes(reg[D], reg[E]);
        case 2: return pairBytes(reg[H], reg[L]);
        case 3: return pairBytes(reg[A], reg[F]);
    }
    throw(std::invalid_argument("Invalid qq-symbol"));
}

void z80::write_qqSymbol(uint8_t qq, uint16_t data) {

    // Get the 2bit constrained conversion of q-symbol to register
    qq &= 0b11;

    switch(qq) {
        case 0: reg[B] = (data >> 8); reg[C] = data; return;
        case 1: reg[D] = (data >> 8); reg[E] = data; return;
        case 2: reg[H] = (data >> 8); reg[L] = data; return;
        case 3: reg[A] = (data >> 8); reg[F] = data; return;
    }
    throw(std::invalid_argument("Invalid qq-symbol"));
}

const char* z80::name_qqSymbol(uint8_t qq) {
    qq &= 0b11;

    switch(qq) {
        case 0: return "BC";
        case 1: return "DE";
        case 2: return "HL";
        case 3: return "AF";
    }
    return "";
}

uint16_t z80::read_ppSymbol(uint8_t pp) {
    pp &= 0b11;

    switch(pp) {
        case 0: return pairBytes(reg[B], reg[C]);
        case 1: return pairBytes(reg[D], reg[E]);
        case 2: return indexRegisterX;
        case 3: return stackPointer;
    }
    throw(std::invalid_argument("Invalid pp-symbol"));
}

void z80::write_ppSymbol(uint8_t pp, uint16_t data) {
    pp &= 0b11;

    switch(pp) {
        case 0: reg[B] = (data >> 8); reg[C] = data; return;
        case 1: reg[D] = (data >> 8); reg[E] = data; return;
        case 2: indexRegisterX = data; return;
        case 3: stackPointer = data; return;
    }
    throw(std::invalid_argument("Invalid pp-symbol"));
}

const char* z80::name_ppSymbol(uint8_t pp) {
    pp &= 0b11;

    switch(pp) {
        case 0: return "BC";
        case 1: return "DE";
        case 2: return "IX";
        case 3: return "SP";
    }
    return "";
}

uint16_t z80::read_rrSymbol(uint8_t rr) {
    rr &= 0b11;

    switch(rr) {
        case 0: return pairBytes(reg[B], reg[C]);
        case 1: return pairBytes(reg[D], reg[E]);
        case 2: return indexRegisterY;
        case 3: return stackPointer;
    }
    throw(std::invalid_argument("Invalid rr-symbol"));
}

void z80::write_rrSymbol(uint8_t rr, uint16_t data) {
    rr &= 0b11;

    switch(rr) {
        case 0: reg[B] = (data >> 8); reg[C] = data; return;
        case 1: reg[D] = (data >> 8); reg[E] = data; return;
        case 2: indexRegisterY = data; return;
        case 3: stackPointer = data; return;
    }
    throw(std::invalid_argument("Invalid rr-symbol"));
}

const char* z80::name_rrSymbol(uint8_t rr) {
    rr &= 0b11;

    switch(rr) {
        case 0: return "BC";
        case 1: return "DE";
        case 2: return "IY";
        case 3: return "SP";
    }
    return "";
}

bool z80::read_cccSymbol(uint8_t ccc) {
    ccc &= 0b0111;

    switch(ccc) {
        case 0: return !getFlag(Zero);
        case 1: return getFlag(Zero);
        case 2: return !getFlag(Carry);
        case 3: return getFlag(Carry);
        case 4: return !getFlag(ParityOverflow);
        case 5: return getFlag(ParityOverflow);
        case 6: return !getFlag(Sign);
        case 7: return getFlag(Sign);
    }
    return false;
}

const char* z80::name_cccSymbol(uint8_t ccc) {
    ccc &= 0b0111;

    switch(ccc) {
        case 0: return "NZ";
        case 1: return "Z";
        case 2: return "NC";
        case 3: return "C";
        case 4: return "PO";
        case 5: return "PE";
        case 6: return "P";
        case 7: return "M";
    }
    return "";
}