#include "common/utilities.h"

uint16_t pairBytes(uint8_t hi, uint8_t lo) {
    return ((uint16_t)hi << 8) + (uint16_t)lo;
}

bool halfCarry8(uint8_t a, uint8_t b, uint8_t c) {
    return ((a & 0x0F) + (b & 0x0F) + (c & 0x0F)) > 0x0F;
}

bool halfBorrow8(uint8_t a, uint8_t b, uint8_t c) {
    return (a & 0x0F) < ((b & 0x0F) + (c & 0x0F));
}

bool carry8(uint8_t a, uint8_t b, uint8_t c) {
    return ((int)a + (int)b + (int)c) > 0xFF;
}

bool borrow8(uint8_t a, uint8_t b, uint8_t c) {
    return a < (b + c);
}

bool overflow8(bool sign, uint8_t a, uint8_t b, uint8_t c) {
    int res;
    if(sign)    res = (int8_t)a - (int8_t)b - (int8_t)c;
    else        res = (int8_t)a + (int8_t)b + (int8_t)c;
    return res > 127 || res < -128;
}

bool halfCarry16(uint16_t a, uint16_t b, uint16_t c) {
    return ((a & 0x0FFF) + (b & 0x0FFF) + (c & 0x0FFF)) > 0x0FFF;
}

bool halfBorrow16(uint16_t a, uint16_t b, uint16_t c) {
    return (a & 0xFFF) < ((b & 0xFFF) + (c & 0xFFF));
}

bool carry16(uint16_t a, uint16_t b, uint16_t c) {
    return ((int)a + (int)b + (int)c) > 0xFFFF;
}

bool borrow16(uint16_t a, uint16_t b, uint16_t c) {
    return a < (b + c);
}

bool overflow16(bool sign, uint16_t a, uint16_t b, uint16_t c) {
    int res;
    if(sign)    res = (int16_t)a - (int16_t)b - (int16_t)c;
    else        res = (int16_t)a + (int16_t)b + (int16_t)c;
    return res > 32767 || res < -32768;
}

uint8_t reverse(uint8_t num) {
    uint8_t res = 0;
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