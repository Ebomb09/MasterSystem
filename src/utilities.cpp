#include "utilities.h"
#include <iostream>

uint16 pairBytes(uint8 hi, uint8 lo) {
    return ((uint16)hi << 8) + (uint16)lo;
}

bool halfCarry8(uint8 a, uint8 b, uint8 c) {
    return ((a & 0x0F) + (b & 0x0F) + (c & 0x0F)) > 0x0F;
}

bool halfBorrow8(uint8 a, uint8 b, uint8 c) {
    return (a & 0x0F) < ((b & 0x0F) + (c & 0x0F));
}

bool carry8(uint8 a, uint8 b, uint8 c) {
    return ((int)a + (int)b + (int)c) > 0xFF;
}

bool borrow8(uint8 a, uint8 b, uint8 c) {
    return a < (b + c);
}

bool overflow8(bool sign, uint8 a, uint8 b, uint8 c) {
    int res;
    if(sign)    res = (sint8)a - (sint8)b - (sint8)c;
    else        res = (sint8)a + (sint8)b + (sint8)c;
    return res > 127 || res < -128;
}

bool halfCarry16(uint16 a, uint16 b, uint16 c) {
    return ((a & 0x0FFF) + (b & 0x0FFF) + (c & 0x0FFF)) > 0x0FFF;
}

bool halfBorrow16(uint16 a, uint16 b, uint16 c) {
    return (a & 0xFFF) < ((b & 0xFFF) + (c & 0xFFF));
}

bool carry16(uint16 a, uint16 b, uint16 c) {
    return ((int)a + (int)b + (int)c) > 0xFFFF;
}

bool borrow16(uint16 a, uint16 b, uint16 c) {
    return a < (b + c);
}

bool overflow16(bool sign, uint16 a, uint16 b, uint16 c) {
    int res;
    if(sign)    res = (sint16)a - (sint16)b - (sint16)c;
    else        res = (sint16)a + (sint16)b + (sint16)c;
    return res > 32767 || res < -32768;
}

void toggleLog(bool toggle) {

    if(toggle)
        std::clog.clear();
    else
        std::clog.setstate(std::iostream::failbit);
}

uint8 reverse(uint8 num) {
    uint8 res = 0;
    if(num & 1) res |= 128;
    if(num & 2) res |= 64;
    if(num & 4) res |= 32;
    if(num & 8) res |= 16;
    if(num & 16) res |= 8;
    if(num & 32) res |= 4;
    if(num & 64) res |= 2;
    if(num & 128) res |= 1;
    return res;
}