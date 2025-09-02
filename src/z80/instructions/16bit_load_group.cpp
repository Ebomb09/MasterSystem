#include "z80/z80.h"
#include "common/utilities.h"
#include <iostream>

int z80::process16BitLoadGroup() {

    uint8_t byte[4] {
        mapper_read(programCounter),
        mapper_read(programCounter+1),
        mapper_read(programCounter+2),
        mapper_read(programCounter+3)
    };
    
    switch(byte[0]) {

        /* LD dd, nn
        
            [ 0 | 0 | d   > | 0 | 0 | 0 | 1 ]
            [ <   -   -   n   -   -   -   > ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b00000001: case 0b00010001: case 0b00100001: case 0b00110001: 
        {
            incrementPC(3);
            uint8_t dd = (byte[0] & 0b00110000) >> 4;
            uint16_t data = pairBytes(byte[2], byte[1]);
            write_ddSymbol(dd, data);
            std::clog << "LD " << name_ddSymbol(dd) << ", " << std::hex << (int)data << "\n";
            return 10;
        }


        /* LD HL, (nn)
        
            [ 0 | 0 | 1 | 0 | 1 | 0 | 1 | 0 ]
            [ <   -   -   n   -   -   -   > ]
            [ <   -   -   n   -   -   -   > ]    
        */
        case 0b00101010:
        {
            incrementPC(3);
            uint16_t addr = pairBytes(byte[2], byte[1]);
            reg[H] = mapper_read(addr+1);
            reg[L] = mapper_read(addr);
            std::clog << "LD HL, (" << std::hex << (int)addr << ")\n";
            return 16;
        }


        /* LD (nn), HL
        
            [ 0 | 0 | 1 | 0 | 0 | 0 | 1 | 0 ]
            [ <   -   -   n   -   -   -   > ]
            [ <   -   -   n   -   -   -   > ]    
        */
        case 0b00100010:
        {
            incrementPC(3);
            uint16_t addr = pairBytes(byte[2], byte[1]);
            mapper_write(addr, reg[L]);
            mapper_write(addr+1, reg[H]);
            std::clog << "LD (" << std::hex << (int)addr << "), HL\n";
            return 16;
        }


        /* LD SP, HL

            [ 1 | 1 | 1 | 1 | 1 | 0 | 0 | 1 ]
        */
        case 0b11111001:
        {
            incrementPC(1);
            stackPointer = pairBytes(reg[H], reg[L]);
            std::clog << "LD SP, HL\n";
            return 6;
        }


        /* PUSH qq

            [ 1 | 1 | q   > | 0 | 1 | 0 | 1 ]
        */
        case 0b11000101: case 0b11010101: case 0b11100101: case 0b11110101:
        {
            incrementPC(1);
            uint8_t qq = (byte[0] & 0b00110000) >> 4;
            uint16_t data = read_qqSymbol(qq);
            PUSH(data);
            std::clog << "PUSH " << name_qqSymbol(qq) << "\n";
            return 11;
        }


        /* POP qq

            [ 1 | 1 | q   > | 0 | 0 | 0 | 1 ]
        */
        case 0b11000001: case 0b11010001: case 0b11100001: case 0b11110001:
        {
            incrementPC(1);
            uint8_t qq = (byte[0] & 0b00110000) >> 4;
            uint16_t data; 
            POP(data);
            write_qqSymbol(qq, data);
            std::clog << "POP " << name_qqSymbol(qq) << "\n";
            return 10;
        }


        /* Load Opcodes
        
            [ 1 | 1 | 1 | 0 | 1 | 1 | 0 | 1 ]
        */
        case 0b11101101: 
        {

            switch(byte[1]) {

                /* LD dd, (nn)

                    [ 0 | 1 | d   > | 1 | 0 | 1 | 1 ]
                    [ <   -   -   n   -   -   -   > ]
                    [ <   -   -   n   -   -   -   > ]
                */
                case 0b01001011: case 0b01011011: case 0b01101011: case 0b01111011:
                {
                    incrementPC(4);
                    uint8_t dd = (byte[1] & 0b00110000) >> 4;
                    uint16_t addr = pairBytes(byte[3], byte[2]);
                    uint16_t data = pairBytes(mapper_read(addr+1), mapper_read(addr)); 
                    write_ddSymbol(dd, data);
                    std::clog << "LD " << name_ddSymbol(dd) << ", (" << std::hex << addr << ")\n";
                    return 20;
                }

                
                /* LD (nn), dd

                    [ 0 | 1 | d   > | 0 | 0 | 1 | 1 ]
                    [ <   -   -   n   -   -   -   > ]
                    [ <   -   -   n   -   -   -   > ]
                */
                case 0b01000011: case 0b01010011: case 0b01100011: case 0b01110011:
                {
                    incrementPC(4);
                    uint8_t dd = (byte[1] & 0b00110000) >> 4;
                    uint16_t addr = pairBytes(byte[3], byte[2]);
                    uint16_t data = read_ddSymbol(dd); 
                    mapper_write(addr, data);
                    mapper_write(addr+1, data >> 8);
                    std::clog << "LD (" << std::hex << (int)addr << "), " << name_ddSymbol(dd) << "\n";
                    return 20;
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
            uint16_t& index = (byte[0] == 0b11011101) ? indexRegisterX : indexRegisterY;

            switch(byte[1]) {

                /* LD IX, nn

                    [ 0 | 0 | 1 | 0 | 0 | 0 | 0 | 1 ]
                    [ <   -   -   n   -   -   -   > ]
                    [ <   -   -   n   -   -   -   > ]
                */
                case 0b00100001:
                {
                    incrementPC(4);
                    index = pairBytes(byte[3], byte[2]);
                    std::clog << "LD IX, " << std::hex << (int)index << "\n";
                    return 14;
                }

                /* LD IX, (nn)

                    [ 0 | 0 | 1 | 0 | 1 | 0 | 1 | 0 ]
                    [ <   -   -   n   -   -   -   > ]
                    [ <   -   -   n   -   -   -   > ]
                */
                case 0b00101010:
                {
                    incrementPC(4);
                    uint16_t addr = pairBytes(byte[3], byte[2]);
                    uint16_t data = pairBytes(mapper_read(addr+1), mapper_read(addr));
                    index = data;
                    std::clog << "LD IX, (" << std::hex << (int)addr << ")\n";
                    return 20;
                }

                /* LD (nn), IX

                    [ 0 | 0 | 1 | 0 | 0 | 0 | 1 | 0 ]
                    [ <   -   -   n   -   -   -   > ]
                    [ <   -   -   n   -   -   -   > ]
                */
                case 0b00100010:
                {
                    incrementPC(4);
                    uint16_t addr = pairBytes(byte[3], byte[2]);
                    mapper_write(addr, index);
                    mapper_write(addr+1, index >> 8);
                    std::clog << "LD (" << std::hex << (int)addr << "), IX\n";
                    return 20;
                }

                /* LD SP, IX
                
                    [ 1 | 1 | 1 | 1 | 1 | 0 | 0 | 1 ]
                */
                case 0b11111001:
                {
                    incrementPC(2);
                    stackPointer = index;
                    std::clog << "LD SP, IX\n";
                    return 10;
                }

                /* PUSH IX

                    [ 1 | 1 | 1 | 0 | 0 | 1 | 0 | 1 ]
                */
                case 0b11100101:
                {
                    incrementPC(2);
                    PUSH(index);
                    std::clog << "PUSH IX\n";
                    return 15;
                }


                /* POP IX

                    [ 1 | 1 | 1 | 0 | 0 | 0 | 0 | 1 ]
                */
                case 0b11100001:
                {
                    incrementPC(2);
                    POP(index);
                    std::clog << "POP IX\n";
                    return 14;
                }
            }
            break;
        }

    }
    return 0;
}