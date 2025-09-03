#ifndef TMS9918A_UTILITIES_H
#define TMS9918A_UTILITIES_H

#include <cstdint>

static inline uint16_t pairBytes(uint8_t hi, uint8_t lo) {
    return ((uint16_t)hi << 8) + (uint16_t)lo;
}

static inline uint8_t reverse(uint8_t num) {
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

#endif