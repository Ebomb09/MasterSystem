#include "SN76489.h"

SN76489::SN76489() {

    // Initialize all muted
    for(int i = 0; i < 4; i ++) {
        reg[i] = 0xF000;
        counter[i] = 0;
        output[i] = 1;
    }
    linearFeedback = 1 << 15;
    outputNoise = 0;
    latchChannel = 0;
    latchType = 0;
}

void SN76489::write(uint8_t byte) {
    uint8_t latch = (byte & 0b10000000) >> 7;
    
    switch(latch) {

        // Data byte
        case 0:
        {

            if(latchType == 0) {

                // Noise
                if(latchChannel == 3) {
                    linearFeedback = 1 << 15;
                    reg[latchChannel] &= 0b1111000000000000;
                    reg[latchChannel] |= (byte & 0b00000111);

                // Tone
                }else {
                    reg[latchChannel] &= 0b1111000000001111;
                    reg[latchChannel] |= (byte & 0b00111111) << 4;
                }

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
            
            if(latchType == 0) {

                // Noise
                if(latchChannel == 3) {
                    linearFeedback = 1 << 15;
                    reg[latchChannel] &= 0b1111000000000000;
                    reg[latchChannel] |= (byte & 0b00000111);

                // Tone
                }else {
                    reg[latchChannel] &= 0b1111001111110000;
                    reg[latchChannel] |= (byte & 0b00001111);
                }

            // Volume
            }else {
                reg[latchChannel] &= 0b0000001111111111;
                reg[latchChannel] |= (byte & 0b00001111) << 12;
            }
            break;
        }
    }
}

void SN76489::cycle() {
    
    // Tones
    for(int i = 0; i < 3; i ++) {
        uint16_t tone     = (reg[i] & 0b0000001111111111);

        if(counter[i] > 0) {
            counter[i] --;

        }else if(counter[i] == 0) {
            counter[i] = tone;
            output[i] *= -1;
        }
    }

    // Noise
    if(counter[3] > 0) {
        counter[3] --;

    }else if(counter[3] == 0) {
        uint8_t noise = (reg[3] & 0b0000000000000100) >> 2;
        uint8_t reset = reg[3] & 0b0000000000000011;

        // Reset value
        switch(reset) {
            case 0: counter[3] = 0x10; break;
            case 1: counter[3] = 0x20; break;
            case 2: counter[3] = 0x40; break;
            case 3: counter[3] = (reg[2] & 0b0000001111111111); break;
        }
        output[3] *= -1;

        if(output[3] == 1) {
            uint16_t in = 0;

            outputNoise = (linearFeedback & 1);

            // White Noise
            if(noise == 1) {
                uint8_t bit1 = (linearFeedback & (1 << 0)) ? 1 : 0;
                uint8_t bit2 = (linearFeedback & (1 << 3)) ? 1 : 0;
                
                if(bit1 ^ bit2)
                    in = (1 << 15);

            // Periodic Noise
            }else {
                
                if(linearFeedback & 1)
                    in = (1 << 15);
            }
            linearFeedback = in | (linearFeedback >> 1);
        }
    }
}

float SN76489::getSample() {
    float mix = 0.f;
    
    // Tones
    for(int i = 0; i < 3; i ++) {
        uint8_t volume    = (reg[i] & 0b1111000000000000) >> 12;
        uint16_t tone     = (reg[i] & 0b0000001111111111);

        if(counter[i] > 0)
            mix += output[i] * (1.0f - volume / 15.f);
    }

    // Noise
    if(counter[3] > 0) {
        uint8_t volume = (reg[3] & 0b1111000000000000) >> 12;
        mix += outputNoise * (1.0f - volume / 15.f);
    }

    return mix / 4.f;
}