#include "psg.h"
#include <iostream>

psg::psg() {

    for(int i = 0; i < 4; i ++) {
        reg[i] = 0;
        counter[i] = 0;
        output[i] = 1;
    }
}

void psg::write(uint8 byte) {
    uint8 latch = (byte & 0b10000000) >> 7;
    
    switch(latch) {

        // Data byte
        case 0:
        {

            // High 6 bits of tone
            if(latchType == 0) {
                reg[latchChannel] &= 0b1111000000001111;
                reg[latchChannel] |= (byte & 0b00111111) << 4;

            // Volume
            }else {
                reg[latchChannel] &= 0b0000001111111111;
                reg[latchChannel] |= (byte & 0b00001111) << 12;
            }
            break;
        }

        // Latch byte
        case 1:
        {
            latchChannel = (byte & 0b01100000) >> 5;
            latchType = (byte & 0b00010000) >> 4;
            
            // Low 4 bits of tone
            if(latchType == 0) {
                reg[latchChannel] &= 0b1111001111110000;
                reg[latchChannel] |= (byte & 0b00001111);

            // Volume
            }else {
                reg[latchChannel] &= 0b0000001111111111;
                reg[latchChannel] |= (byte & 0b00001111) << 12;
            }
            break;
        }
    }
}

void psg::cycle() {
    
    for(int i = 0; i < 3; i ++) {
        uint8 volume    = (reg[i] & 0b1111000000000000) >> 12;
        uint16 tone     = (reg[i] & 0b0000001111111111);

        if(counter[i] > 0) {
            counter[i] --;

        }else if(counter[i] == 0) {
            counter[i] = tone;
            output[i] *= -1;
        }
    }
}

float psg::getSample() {
    float mix = 0.f;
    
    for(int i = 0; i < 4; i ++) {
        uint8 volume    = (reg[i] & 0b1111000000000000) >> 12;
        uint16 tone     = (reg[i] & 0b0000001111111111);

        if(counter[i] > 0)
            mix += output[i] * (1.0f - volume / 15.f);
    }
    return mix / 4.f;
}