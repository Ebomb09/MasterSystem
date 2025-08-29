#ifndef SEGA_PSG_H
#define SEGA_PSG_H

#include "utilities.h"

struct psg {

    psg();

    uint16 reg[4];
    uint16 counter[4];
    int output[4];
    uint16 linearFeedback;
    int outputNoise;

    uint8 latchChannel;
    uint8 latchType;

    void write(uint8 byte);
    void cycle();
    float getSample();
};

#endif