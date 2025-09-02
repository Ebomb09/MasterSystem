#include "z80/z80.h"
#include "common/utilities.h"
#include <iostream>

int z80::processJumpGroup() {

    uint8_t byte[4] {
        mapper_read(programCounter),
        mapper_read(programCounter+1),
        mapper_read(programCounter+2),
        mapper_read(programCounter+3)
    };

    switch(byte[0]) {

        /* JP nn
        
            [ 1 | 1 | 0 | 0 | 0 | 0 | 1 | 1 ]
            [ <   -   -   n   -   -   -   > ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b11000011:
        {
            incrementPC(3);
            uint16_t addr = pairBytes(byte[2], byte[1]);
            programCounter = addr;

            std::clog << "JP " << std::hex << (int)addr << "\n";
            return 10;
        }


        /* JP cc, nn
        
            [ 1 | 1 | <   c   > | 0 | 1 | 0 ]
            [ <   -   -   n   -   -   -   > ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b11000010: case 0b11001010: case 0b11010010: case 0b11011010: case 0b11100010: case 0b11101010: case 0b11110010: case 0b11111010:
        {
            incrementPC(3);
            uint8_t ccc = (byte[0] & 0b00111000) >> 3;
            uint16_t addr = pairBytes(byte[2], byte[1]);

            if(read_cccSymbol(ccc)) {
                programCounter = addr;
            }
            std::clog << "JP " << name_cccSymbol(ccc) << ", " << std::hex << (int)addr << "\n";
            return 10;
        }


        /* JR e
        
            [ 0 | 0 | 0 | 1 | 1 | 0 | 0 | 0 ]
            [ <   -   -   e   -   -   -   > ]
        */
        case 0b00011000:
        {
            incrementPC(2);
            int8_t offset = (int8_t)byte[1];
            programCounter += offset;
            std::clog << "JR " << (int)offset << "\n";
            return 12;
        }


        /* JR C, e
        
            [ 0 | 0 | 1 | 1 | 1 | 0 | 0 | 0 ]
            [ <   -   -   e   -   -   -   > ]
        */
        case 0b00111000:
        {
            incrementPC(2);
            int8_t offset = (int8_t)byte[1];

            std::clog << "JR C, " << (int)offset << "\n";

            if(getFlag(Carry)) {
                programCounter += offset;
                return 12;
            }
            return 7;
        }


        /* JR NC, e
        
            [ 0 | 0 | 1 | 1 | 0 | 0 | 0 | 0 ]
            [ <   -   -   e   -   -   -   > ]
        */
        case 0b00110000:
        {
            incrementPC(2);
            int8_t offset = (int8_t)byte[1];

            std::clog << "JR NC, " << (int)offset << "\n";

            if(!getFlag(Carry)) {
                programCounter += offset;
                return 12;
            }
            return 7;
        }


        /* JR Z, e
        
            [ 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 ]
            [ <   -   -   e   -   -   -   > ]
        */
        case 0b00101000:
        {
            incrementPC(2);
            int8_t offset = (int8_t)byte[1];

            std::clog << "JR Z, " << (int)offset << "\n";

            if(getFlag(Zero)) {
                programCounter += offset;
                return 12;
            }
            return 7;
        }


        /* JR NZ, e
        
            [ 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 ]
            [ <   -   -   e   -   -   -   > ]
        */
        case 0b00100000:
        {
            incrementPC(2);
            int8_t offset = (int8_t)byte[1];

            std::clog << "JR NZ, " << (int)offset << "\n";

            if(!getFlag(Zero)) {
                programCounter += offset;
                return 12;
            }
            return 7;
        }


        /* JP (HL)
        
            [ 1 | 1 | 1 | 0 | 1 | 0 | 0 | 1 ]
        */
        case 0b11101001:
        {
            incrementPC(1);
            uint16_t addr = pairBytes(reg[H], reg[L]);
            programCounter = addr;
            std::clog << "JP (HL)\n";
            return 4;
        }


        /* DJNZ, e
        
            [ 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 ]
            [ <   -   -   e   -   -   -   > ]
        */
        case 0b00010000:
        {
            incrementPC(2);
            reg[B] = reg[B] - 1;
            int8_t offset = (int8_t)byte[1];

            std::clog << "DJNZ, " << (int)offset << "\n";

            if(reg[B] == 0) {
                return 8;

            }else {
                programCounter += offset;
                return 13;
            }
        }


        /* IX Opcodes
        
            [ 1 | 1 | 0 | 1 | 1 | 1 | 0 | 1 ]
        */
        case 0b11011101:
        {

            switch(byte[1]) {

                /* JP (IX)
                
                    [ 1 | 1 | 1 | 0 | 1 | 0 | 0 | 1 ]
                */
                case 0b11101001:
                {
                    incrementPC(2);
                    programCounter = indexRegisterX;
                    std::clog << "JP, (IX)\n";
                    return 8;
                }
            }
            break;
        }


        /* IY Opcodes
        
            [ 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 ]
        */
        case 0b11111101:
        {

            switch(byte[1]) {

                /* JP (IY)
                
                    [ 1 | 1 | 1 | 0 | 1 | 0 | 0 | 1 ]
                */
                case 0b11101001:
                {
                    incrementPC(2);
                    programCounter = indexRegisterY;
                    std::clog << "JP, (IY)\n";
                    return 8;
                }
            }
            break;
        }
    }
    return 0;
}