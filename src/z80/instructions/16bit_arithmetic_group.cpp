#include "../z80.h"
#include <iostream>

int z80::process16BitArithmeticGroup() {

    uint8 byte[4] {
        mapper_read(programCounter),
        mapper_read(programCounter+1),
        mapper_read(programCounter+2),
        mapper_read(programCounter+3)
    };

    switch(byte[0]) {

        /* ADD HL, ss
        
            [0 | 0 | s   > | 1 | 0 | 0 | 1 ]
        */
        case 0b00001001: case 0b00011001: case 0b00101001: case 0b00111001:
        {
            incrementPC(1);
            uint8 ss = (byte[0] & 0b00110000) >> 4;
            uint16 a = pairBytes(reg[H], reg[L]);
            uint16 b = read_ssSymbol(ss);
            ADD16(a, b);
            reg[H] = a >> 8;
            reg[L] = a; 
            std::clog << "ADD HL, " << name_ssSymbol(ss) << "\n";
            return 11;
        }


        /*  INC ss

            [ 0 | 0 | s   > | 0 | 0 | 1 | 1 ]
        */
        case 0b00000011: case 0b00010011: case 0b00100011: case 0b00110011: 
        {
            incrementPC(1);
            uint8 ss = (byte[0] & 0b00110000) >> 4;
            uint16 a = read_ssSymbol(ss);
            INC16(a);
            write_ssSymbol(ss, a);
            std::clog << "INC " << name_ssSymbol(ss) << "\n";
            return 6;
        }


        /*  DEC ss

            [ 0 | 0 | s   > | 1 | 0 | 1 | 1 ]
        */
        case 0b00001011: case 0b00011011: case 0b00101011: case 0b00111011: 
        {
            incrementPC(1);
            uint8 ss = (byte[0] & 0b00110000) >> 4;
            uint16 a = read_ssSymbol(ss);
            DEC16(a);
            write_ssSymbol(ss, a);
            std::clog << "DEC " << name_ssSymbol(ss) << "\n";
            return 6;
        }


        /* Load Opcodes
        
            [ 1 | 1 | 1 | 0 | 1 | 1 | 0 | 1 ]
        */
        case 0b11101101:
        {

            switch(byte[1]) {

                /* ADC HL, ss

                    [ 0 | 1 | s   > | 1 | 0 | 1 | 0 ]
                */
                case 0b01001010: case 0b01011010: case 0b01101010: case 0b01111010:
                {
                    incrementPC(2);
                    uint8 ss = (byte[1] & 0b00110000) >> 4;
                    uint16 a = pairBytes(reg[H], reg[L]);
                    uint16 b = read_ssSymbol(ss);
                    ADC16(a, b);
                    reg[H] = a >> 8;
                    reg[L] = a; 
                    std::clog << "ADC HL, " << name_ssSymbol(ss) << "\n";
                    return 15;
                }


                /* SBC HL, ss

                    [ 0 | 1 | s   > | 0 | 0 | 1 | 0 ]
                */
                case 0b01000010: case 0b01010010: case 0b01100010: case 0b01110010:
                {
                    incrementPC(2);
                    uint8 ss = (byte[1] & 0b00110000) >> 4;
                    uint16 a = pairBytes(reg[H], reg[L]);
                    uint16 b = read_ssSymbol(ss);
                    SBC16(a, b);
                    reg[H] = a >> 8;
                    reg[L] = a; 
                    std::clog << "SBC HL, " << name_ssSymbol(ss) << "\n";
                    return 15;
                }
            }
            break;
        }

        /* IX Opcodes

            [ 1 | 1 | x | 1 | 1 | 1 | 0 | 1 ]
            IX = 0
            IY = 1
        */
        case 0b11011101: case 0b11111101:
        {
            uint16& index = (byte[0] == 0b11011101) ? indexRegisterX : indexRegisterY;

            switch(byte[1]) {

                /* ADD IX, pp
                
                    [ 0 | 0 | p   > | 1 | 0 | 0 | 1 ]
                */
                case 0b00001001: case 0b00011001: case 0b00101001: case 0b00111001:
                {
                    incrementPC(2);
                    int pp = (byte[1] & 0b00110000) >> 4;

                    if(byte[0] == 0b11011101)
                        ADD16(index, read_ppSymbol(pp));
                    else
                        ADD16(index, read_rrSymbol(pp));

                    std::clog << "ADD IX, " << name_ppSymbol(pp) << "\n";
                    return 15;
                }


                /*  INC IX

                    [ 0 | 0 | 1 | 0 | 0 | 0 | 1 | 1 ]
                */
                case 0b00100011: 
                {
                    incrementPC(2);
                    INC16(index);
                    std::clog << "INC IX\n";
                    return 10;
                }


                /*  DEC IX

                    [ 0 | 0 | 1 | 0 | 1 | 0 | 1 | 1 ]
                */
                case 0b00101011: 
                {
                    incrementPC(2);
                    DEC16(index);
                    std::clog << "DEC IX\n";
                    return 10;
                }
            }
            break;
        }        
    }
    return 0;
}