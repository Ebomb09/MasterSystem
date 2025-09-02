#ifndef SEGA_PSG_H
#define SEGA_PSG_H

#include <cstdint>

struct psg {

    psg();

    uint16_t reg[4];
    uint16_t counter[4];
    int output[4];
    uint16_t linearFeedback;
    int outputNoise;

    uint8_t latchChannel;
    uint8_t latchType;

    void write(uint8_t byte);
    void cycle();
    float getSample();
};

#endif