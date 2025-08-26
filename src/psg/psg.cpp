#include "psg.h"

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
                reg[latchChannel] |= (byte & 0x0F) << 12;
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
                reg[latchChannel] |= (byte & 0x0F) << 12;
            }
            break;
        }
    }
}