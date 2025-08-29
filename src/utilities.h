#ifndef SMS_UTIL_H
#define SMS_UTIL_H

#include <cstdio>

using uint8 = unsigned char;
using sint8 = signed char;
using uint16 = unsigned short;
using sint16 = signed short;

enum DeviceType {
    MASTER_SYSTEM_NTSC,
    MASTER_SYSTEM_PAL,
    GAME_GEAR
};

uint16 pairBytes(uint8 hi, uint8 lo);

bool halfCarry8(uint8 a, uint8 b, uint8 c = 0);
bool carry8(uint8 a, uint8 b, uint8 c = 0);
bool halfBorrow8(uint8 a, uint8 b, uint8 c = 0);
bool borrow8(uint8 a, uint8 b, uint8 c = 0);
bool overflow8(bool sign, uint8 a, uint8 b, uint8 c = 0);

bool halfCarry16(uint16 a, uint16 b, uint16 c = 0);
bool carry16(uint16 a, uint16 b, uint16 c = 0);
bool halfBorrow16(uint16 a, uint16 b, uint16 c = 0);
bool borrow16(uint16 a, uint16 b, uint16 c = 0);
bool overflow16(bool sign, uint16 a, uint16 b, uint16 c = 0);

void toggleLog(bool toggle);

uint8 reverse(uint8 num);

#endif