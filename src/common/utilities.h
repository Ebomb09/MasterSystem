#ifndef COMMON_UTILITIES_H
#define COMMON_UTILITIES_H

#include <cstdint>

uint16_t pairBytes(uint8_t hi, uint8_t lo);
uint8_t reverse(uint8_t num);

bool halfCarry8(uint8_t a, uint8_t b, uint8_t c = 0);
bool carry8(uint8_t a, uint8_t b, uint8_t c = 0);
bool halfBorrow8(uint8_t a, uint8_t b, uint8_t c = 0);
bool borrow8(uint8_t a, uint8_t b, uint8_t c = 0);
bool overflow8(bool sign, uint8_t a, uint8_t b, uint8_t c = 0);

bool halfCarry16(uint16_t a, uint16_t b, uint16_t c = 0);
bool carry16(uint16_t a, uint16_t b, uint16_t c = 0);
bool halfBorrow16(uint16_t a, uint16_t b, uint16_t c = 0);
bool borrow16(uint16_t a, uint16_t b, uint16_t c = 0);
bool overflow16(bool sign, uint16_t a, uint16_t b, uint16_t c = 0);

#endif