#include "Z80.h"
#include "utilities.h"
#include <iostream>

int Z80::processBitSetResetTest() {

    uint8_t byte[4] {
        mapper_read(programCounter),
        mapper_read(programCounter+1),
        mapper_read(programCounter+2),
        mapper_read(programCounter+3)
    };

    switch(byte[0]) {

        /* Shift Opcodes
        
            [ 1 | 1 | 0 | 0 | 1 | 0 | 1 | 1 ]
        */
        case 0b11001011:
        {

            switch(byte[1]) {

                /* BIT b, r
                
                    [ 0 | 1 | <   b   > | <   r   > ]
                */
                case 0b01000111: case 0b01001111: case 0b01010111: case 0b01011111: case 0b01100111: case 0b01101111: case 0b01110111: case 0b01111111:
                case 0b01000000: case 0b01001000: case 0b01010000: case 0b01011000: case 0b01100000: case 0b01101000: case 0b01110000: case 0b01111000:
                case 0b01000001: case 0b01001001: case 0b01010001: case 0b01011001: case 0b01100001: case 0b01101001: case 0b01110001: case 0b01111001:
                case 0b01000010: case 0b01001010: case 0b01010010: case 0b01011010: case 0b01100010: case 0b01101010: case 0b01110010: case 0b01111010:
                case 0b01000011: case 0b01001011: case 0b01010011: case 0b01011011: case 0b01100011: case 0b01101011: case 0b01110011: case 0b01111011:
                case 0b01000100: case 0b01001100: case 0b01010100: case 0b01011100: case 0b01100100: case 0b01101100: case 0b01110100: case 0b01111100:
                case 0b01000101: case 0b01001101: case 0b01010101: case 0b01011101: case 0b01100101: case 0b01101101: case 0b01110101: case 0b01111101:
                {
                    incrementPC(2);
                    uint8_t rrr = byte[1] & 0b00000111;
                    uint8_t bbb = (byte[1] & 0b00111000) >> 3;
                    BIT(1 << bbb, read_rrrSymbol(rrr));
                    std::clog << "BIT " << (int)bbb << ", " << name_rrrSymbol(rrr) << "\n";
                    return 8;
                }


                /* SET b, r
                
                    [ 1 | 1 | <   b   > | <   r   > ]
                */
                case 0b11000111: case 0b11001111: case 0b11010111: case 0b11011111: case 0b11100111: case 0b11101111: case 0b11110111: case 0b11111111:
                case 0b11000000: case 0b11001000: case 0b11010000: case 0b11011000: case 0b11100000: case 0b11101000: case 0b11110000: case 0b11111000:
                case 0b11000001: case 0b11001001: case 0b11010001: case 0b11011001: case 0b11100001: case 0b11101001: case 0b11110001: case 0b11111001:
                case 0b11000010: case 0b11001010: case 0b11010010: case 0b11011010: case 0b11100010: case 0b11101010: case 0b11110010: case 0b11111010:
                case 0b11000011: case 0b11001011: case 0b11010011: case 0b11011011: case 0b11100011: case 0b11101011: case 0b11110011: case 0b11111011:
                case 0b11000100: case 0b11001100: case 0b11010100: case 0b11011100: case 0b11100100: case 0b11101100: case 0b11110100: case 0b11111100:
                case 0b11000101: case 0b11001101: case 0b11010101: case 0b11011101: case 0b11100101: case 0b11101101: case 0b11110101: case 0b11111101:
                {
                    incrementPC(2);
                    uint8_t rrr = byte[1] & 0b00000111;
                    uint8_t bbb = (byte[1] & 0b00111000) >> 3;
                    uint8_t data = read_rrrSymbol(rrr);
                    SET(1 << bbb, data);
                    write_rrrSymbol(rrr, data);
                    std::clog << "SET " << (int)bbb << ", " << name_rrrSymbol(rrr) << "\n";
                    return 8;
                }


                /* RES b, r
                
                    [ 1 | 0 | <   b   > | <   r   > ]
                */
                case 0b10000111: case 0b10001111: case 0b10010111: case 0b10011111: case 0b10100111: case 0b10101111: case 0b10110111: case 0b10111111:
                case 0b10000000: case 0b10001000: case 0b10010000: case 0b10011000: case 0b10100000: case 0b10101000: case 0b10110000: case 0b10111000:
                case 0b10000001: case 0b10001001: case 0b10010001: case 0b10011001: case 0b10100001: case 0b10101001: case 0b10110001: case 0b10111001:
                case 0b10000010: case 0b10001010: case 0b10010010: case 0b10011010: case 0b10100010: case 0b10101010: case 0b10110010: case 0b10111010:
                case 0b10000011: case 0b10001011: case 0b10010011: case 0b10011011: case 0b10100011: case 0b10101011: case 0b10110011: case 0b10111011:
                case 0b10000100: case 0b10001100: case 0b10010100: case 0b10011100: case 0b10100100: case 0b10101100: case 0b10110100: case 0b10111100:
                case 0b10000101: case 0b10001101: case 0b10010101: case 0b10011101: case 0b10100101: case 0b10101101: case 0b10110101: case 0b10111101:
                {
                    incrementPC(2);
                    uint8_t rrr = byte[1] & 0b00000111;
                    uint8_t bbb = (byte[1] & 0b00111000) >> 3;
                    uint8_t data = read_rrrSymbol(rrr);
                    RES(1 << bbb, data);
                    write_rrrSymbol(rrr, data);
                    std::clog << "RES " << (int)bbb << ", " << name_rrrSymbol(rrr) << "\n";
                    return 8;
                }


                /* BIT b, (HL)
                
                    [ 0 | 1 | <   b   > | 1 | 1 | 0 ]
                */
                case 0b01000110: case 0b01001110: case 0b01010110: case 0b01011110: case 0b01100110: case 0b01101110: case 0b01110110: case 0b01111110:
                {
                    incrementPC(2);
                    uint16_t addr = pairBytes(reg[H], reg[L]);
                    uint8_t bbb = (byte[1] & 0b00111000) >> 3;
                    BIT(1 << bbb, mapper_read(addr));
                    std::clog << "BIT " << (int)bbb << ", (HL)\n";
                    return 12;
                }


                /* SET b, (HL)
                
                    [ 1 | 1 | <   b   > | 1 | 1 | 0 ]
                */
                case 0b11000110: case 0b11001110: case 0b11010110: case 0b11011110: case 0b11100110: case 0b11101110: case 0b11110110: case 0b11111110:
                {
                    incrementPC(2);
                    uint16_t addr = pairBytes(reg[H], reg[L]);
                    uint8_t bbb = (byte[1] & 0b00111000) >> 3;
                    uint8_t data = mapper_read(addr);
                    SET(1 << bbb, data);
                    mapper_write(addr, data);
                    std::clog << "SET " << (int)bbb << ", (HL)\n";
                    return 15;
                }


                /* RES b, (HL)
                
                    [ 1 | 0 | <   b   > | 1 | 1 | 0 ]
                */
                case 0b10000110: case 0b10001110: case 0b10010110: case 0b10011110: case 0b10100110: case 0b10101110: case 0b10110110: case 0b10111110:
                {
                    incrementPC(2);
                    uint16_t addr = pairBytes(reg[H], reg[L]);
                    uint8_t bbb = (byte[1] & 0b00111000) >> 3;
                    uint8_t data = mapper_read(addr);
                    RES(1 << bbb, data);
                    mapper_write(addr, data);
                    std::clog << "RES " << (int)bbb << ", (HL)\n";
                    return 15;
                }
            }
            break;
        }
    
        /* IX/IY Opcodes
        
            [ 1 | 1 | x | 1 | 1 | 1 | 0 | 1 ]
            IX = 0
            IY = 0
        */
        case 0b11011101: case 0b11111101:
        {
            uint16_t& index = (byte[0] == 0b11011101) ? indexRegisterX : indexRegisterY;

            /* Shift Opcodes

                [ 1 | 1 | 0 | 0 | 1 | 0 | 1 | 1 ]
            */
            if(byte[1] == 0b11001011) {

                switch(byte[3]) {

                    /* BIT b, (IX+d)
                    
                        [ 0 | 1 | <   b   > | 1 | 1 | 0 ]
                    */
                    case 0b01000110: case 0b01001110: case 0b01010110: case 0b01011110: case 0b01100110: case 0b01101110: case 0b01110110: case 0b01111110:
                    {
                        incrementPC(4);
                        uint16_t addr = index + (int8_t)byte[2];
                        uint8_t bbb = (byte[3] & 0b00111000) >> 3;
                        BIT(1 << bbb, mapper_read(addr));
                        std::clog << "BIT " << (int)bbb << ", (IX+d)\n";
                        return 20;
                    }


                    /* SET b, (IX+d)
                    
                        [ 1 | 1 | <   b   > | 1 | 1 | 0 ]
                    */
                    case 0b11000110: case 0b11001110: case 0b11010110: case 0b11011110: case 0b11100110: case 0b11101110: case 0b11110110: case 0b11111110:
                    {
                        incrementPC(4);
                        uint16_t addr = index + (int8_t)byte[2];
                        uint8_t bbb = (byte[3] & 0b00111000) >> 3;
                        uint8_t data = mapper_read(addr);
                        SET(1 << bbb, data);
                        mapper_write(addr, data);
                        std::clog << "SET " << (int)bbb << ", (IX+d)\n";
                        return 23;
                    }


                    /* RES b, (IX+d)
                    
                        [ 1 | 0 | <   b   > | 1 | 1 | 0 ]
                    */
                    case 0b10000110: case 0b10001110: case 0b10010110: case 0b10011110: case 0b10100110: case 0b10101110: case 0b10110110: case 0b10111110:
                    {
                        incrementPC(4);
                        uint16_t addr = index + (int8_t)byte[2];
                        uint8_t bbb = (byte[3] & 0b00111000) >> 3;
                        uint8_t data = mapper_read(addr);
                        RES(1 << bbb, data);
                        mapper_write(addr, data);
                        std::clog << "RES " << (int)bbb << ", (IX+d)\n";
                        return 23;
                    }
                }
            }
            break;
        }
    }
    return 0;
}