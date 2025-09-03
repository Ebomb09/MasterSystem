#ifndef SN76489_PROCESSOR_H
#define SN76489_PROCESSOR_H

#include <cstdint>

struct SN76489 {

    SN76489();

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