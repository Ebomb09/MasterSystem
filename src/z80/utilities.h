#ifndef Z80_UTILITIES_H
#define Z80_UTILITIES_H

#include <cstdint>

static inline uint16_t pairBytes(uint8_t hi, uint8_t lo) {
    return ((uint16_t)hi << 8) + (uint16_t)lo;
}

static inline bool halfCarry8(uint8_t a, uint8_t b, uint8_t c = 0) {
    return ((a & 0x0F) + (b & 0x0F) + (c & 0x0F)) > 0x0F;
}

static inline bool halfBorrow8(uint8_t a, uint8_t b, uint8_t c = 0) {
    return (a & 0x0F) < ((b & 0x0F) + (c & 0x0F));
}

static inline bool carry8(uint8_t a, uint8_t b, uint8_t c = 0) {
    return ((int)a + (int)b + (int)c) > 0xFF;
}

static inline bool borrow8(uint8_t a, uint8_t b, uint8_t c = 0) {
    return a < (b + c);
}

static inline bool overflow8(bool sign, uint8_t a, uint8_t b, uint8_t c = 0) {
    int res;
    if(sign)    res = (int8_t)a - (int8_t)b - (int8_t)c;
    else        res = (int8_t)a + (int8_t)b + (int8_t)c;
    return res > 127 || res < -128;
}

static inline bool halfCarry16(uint16_t a, uint16_t b, uint16_t c = 0) {
    return ((a & 0x0FFF) + (b & 0x0FFF) + (c & 0x0FFF)) > 0x0FFF;
}

static inline bool halfBorrow16(uint16_t a, uint16_t b, uint16_t c = 0) {
    return (a & 0xFFF) < ((b & 0xFFF) + (c & 0xFFF));
}

static inline bool carry16(uint16_t a, uint16_t b, uint16_t c = 0) {
    return ((int)a + (int)b + (int)c) > 0xFFFF;
}

static inline bool borrow16(uint16_t a, uint16_t b, uint16_t c = 0) {
    return a < (b + c);
}

static inline bool overflow16(bool sign, uint16_t a, uint16_t b, uint16_t c = 0) {
    int res;
    if(sign)    res = (int16_t)a - (int16_t)b - (int16_t)c;
    else        res = (int16_t)a + (int16_t)b + (int16_t)c;
    return res > 32767 || res < -32768;
}

#endif