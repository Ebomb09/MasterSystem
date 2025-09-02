#include "z80/z80.h"
#include "common/utilities.h"
#include <iostream>

int z80::processCallReturnGroup() {

    uint8_t byte[4] {
        mapper_read(programCounter),
        mapper_read(programCounter+1),
        mapper_read(programCounter+2),
        mapper_read(programCounter+3)
    };

    switch(byte[0]) {

        /* CALL nn
        
            [ 1 | 1 | 0 | 0 | 1 | 1 | 0 | 1 ]
            [ <   -   -   n   -   -   -   > ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b11001101:
        {
            incrementPC(3);
            uint16_t addr = pairBytes(byte[2], byte[1]);
            CALL(addr);
            std::clog << "CALL " << std::hex << (int)addr << "\n";
            return 17;
        }


        /* CALL cc, nn
        
            [ 1 | 1 | c | c | c | 1 | 0 | 0 ]
            [ <   -   -   n   -   -   -   > ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b11000100: case 0b11001100: case 0b11010100: case 0b11011100: case 0b11100100: case 0b11101100: case 0b11110100: case 0b11111100:
        {
            incrementPC(3);
            uint16_t addr = pairBytes(byte[2], byte[1]);
            uint8_t ccc = (byte[0] & 0b00111000) >> 3;

            std::clog << "CALL " << name_cccSymbol(ccc) << ", " << std::hex << (int)addr << "\n";

            if(read_cccSymbol(ccc)) {
                CALL(addr);
                return 17;
            }
            return 10;
        }


        /* RET
        
            [ 1 | 1 | 0 | 0 | 1 | 0 | 0 | 1 ]
        */
        case 0b11001001:
        {
            incrementPC(1);
            RET();
            std::clog << "RET\n";
            return 10;
        }


        /* RET cc
        
            [ 1 | 1 | <   c   > | 0 | 0 | 0 ]
        */
        case 0b11000000: case 0b11001000: case 0b11010000: case 0b11011000: case 0b11100000: case 0b11101000: case 0b11110000: case 0b11111000:
        {
            incrementPC(1);
            uint8_t ccc = (byte[0] & 0b00111000) >> 3;
            std::clog << "RET " << name_cccSymbol(ccc) << "\n";

            if(read_cccSymbol(ccc)) {
                RET();
                return 11;
            }
            return 5;
        }


        /* RST p
        
            [ 1 | 1 | <   t   > | 1 | 1 | 1 ]
        */
        case 0b11000111: case 0b11001111: case 0b11010111: case 0b11011111: case 0b11100111: case 0b11101111: case 0b11110111: case 0b11111111:
        {
            incrementPC(1);
            uint8_t ttt = (byte[0] & 0b00111000) >> 3;
            RST(ttt);
            std::clog << "RST " << std::hex << (int)ttt << "\n";
            return 11;
        }


        /* Load Opcodes
        
            [ 1 | 1 | 1 | 0 | 1 | 1 | 0 | 1 ]
        */
        case 0b11101101:
        {

            switch(byte[1]) {

                /* RETI
                
                    [ 0 | 1 | 0 | 0 | 1 | 1 | 0 | 1 ]
                */
                case 0b01001101:
                {
                    incrementPC(2);
                    RETI();
                    std::clog << "RETI\n";
                    return 14;
                }


                /* RETN
                
                    [ 0 | 1 | 0 | 0 | 0 | 1 | 0 | 1 ]
                */
                case 0b01000101:
                {
                    incrementPC(2);
                    RETN();
                    std::clog << "RETN\n";
                    return 14;
                }
            }
            break;
        }
    }
    return 0;
}