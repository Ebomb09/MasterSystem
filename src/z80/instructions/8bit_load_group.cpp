#include "z80/z80.h"
#include "common/utilities.h"
#include <iostream>

int z80::process8BitLoadGroup() {

    uint8_t byte[4] {
        mapper_read(programCounter),
        mapper_read(programCounter+1),
        mapper_read(programCounter+2),
        mapper_read(programCounter+3)
    };
    
    switch(byte[0]) {

        /* LD r, r'

            [ 0 | 1 | <   r   > | <   r'  > ]
        */
        case 0b01111111: case 0b01000111: case 0b01001111: case 0b01010111: case 0b01011111: case 0b01100111: case 0b01101111:
        case 0b01111000: case 0b01000000: case 0b01001000: case 0b01010000: case 0b01011000: case 0b01100000: case 0b01101000:
        case 0b01111001: case 0b01000001: case 0b01001001: case 0b01010001: case 0b01011001: case 0b01100001: case 0b01101001:
        case 0b01111010: case 0b01000010: case 0b01001010: case 0b01010010: case 0b01011010: case 0b01100010: case 0b01101010:
        case 0b01111011: case 0b01000011: case 0b01001011: case 0b01010011: case 0b01011011: case 0b01100011: case 0b01101011:
        case 0b01111100: case 0b01000100: case 0b01001100: case 0b01010100: case 0b01011100: case 0b01100100: case 0b01101100:
        case 0b01111101: case 0b01000101: case 0b01001101: case 0b01010101: case 0b01011101: case 0b01100101: case 0b01101101:
        {
            incrementPC(1);
            uint8_t rrr = (byte[0] & 0b00111000) >> 3;
            uint8_t rrr_p = byte[0] & 0b00000111;
            write_rrrSymbol(rrr, read_rrrSymbol(rrr_p));
            std::clog << "LD " << name_rrrSymbol(rrr) <<", " << name_rrrSymbol(rrr_p) << "\n";
            return 4;
        }


        /* LD r,n

            [ 0 | 0 | <   r   > | 1 | 1 | 0 ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b00111110: case 0b00000110: case 0b00001110: case 0b00010110: case 0b00011110: case 0b00100110: case 0b00101110:
        {
            incrementPC(2);
            uint8_t rrr = (byte[0] & 0b00111000) >> 3;
            write_rrrSymbol(rrr, byte[1]);
            std::clog << "LD " << name_rrrSymbol(rrr) <<", " << std::hex << (int)byte[1] << "\n";
            return 7;
        }


        /* LD r, (HL)
        
            [ 0 | 1 | <   r  > | 1 | 1 | 0 ]
        */
        case 0b01111110: case 0b01000110: case 0b01001110: case 0b01010110: case 0b01011110: case 0b01100110: case 0b01101110: 
        {
            incrementPC(1);
            uint8_t rrr = (byte[0] & 0b00111000) >> 3;
            uint16_t addr = pairBytes(reg[H], reg[L]);
            write_rrrSymbol(rrr, mapper_read(addr));
            std::clog << "LD " << name_rrrSymbol(rrr) << ", (HL) \n";
            return 7;
        }


        /* LD (HL), r
        
            [ 0 | 1 | 1 | 1 | 0 | <  r  > ]
        */
        case 0b1110111: case 0b1110000: case 0b1110001: case 0b1110010: case 0b1110011: case 0b1110100: case 0b1110101:
        {
            incrementPC(1);
            uint8_t rrr = byte[0] & 0b00000111;
            uint16_t addr = pairBytes(reg[H], reg[L]);
            mapper_write(addr, read_rrrSymbol(rrr));
            std::clog << "LD (HL), " << name_rrrSymbol(rrr) << " \n";
            return 7;
        }

        
        /* LD (HL), n
        
            [ 0 | 0 | 1 | 1 | 0 | 1 | 1 | 0 ]
            [ <   -   -   n   -   -   -   - ]
        */
        case 0b00110110:
        {
            incrementPC(2);
            uint16_t addr = pairBytes(reg[H], reg[L]);
            mapper_write(addr, byte[1]);
            std::clog << "LD (HL), " << (int)byte[1] << "\n";
            return 10;
        }


        /* LD A, (BC)

            [ 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 ]
        */
        case 0b00001010:
        {
            incrementPC(1);
            uint16_t addr = pairBytes(reg[B], reg[C]);
            reg[A] = mapper_read(addr);
            std::clog << "LD A, (BC)\n";
            return 7;
        }


        /* LD (BC), A

            [ 0 | 0 | 0 | 0 | 1 | 0 | 1 | 0 ]
        */
        case 0b00000010:
        {
            incrementPC(1);
            uint16_t addr = pairBytes(reg[B], reg[C]);
            mapper_write(addr, reg[A]);
            std::clog << "LD (BC), A\n";
            return 7;
        }


        /* LD A, (DE)

            [ 0 | 0 | 0 | 1 | 1 | 0 | 1 | 0 ]
        */
        case 0b00011010:
        {
            incrementPC(1);
            uint16_t addr = pairBytes(reg[D], reg[E]);
            reg[A] = mapper_read(addr);
            std::clog << "LD A, (DE)\n";
            return 7;
        }


        /* LD (DE), A

            [ 0 | 0 | 0 | 1 | 0 | 0 | 1 | 0 ]
        */
        case 0b00010010:
        {
            incrementPC(1);
            uint16_t addr = pairBytes(reg[D], reg[E]);
            mapper_write(addr, reg[A]);
            std::clog << "LD (DE), A\n";
            return 7;
        }


        /* LD A, (nn)

            [ 0 | 0 | 1 | 1 | 1 | 0 | 1 | 0 ]
            [ <   -   -   n   -   -   -   > ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b00111010:
        {
            incrementPC(3);
            uint16_t addr = pairBytes(byte[2], byte[1]);
            reg[A] = mapper_read(addr);
            std::clog << "LD A, (" << std::hex << (int)addr << ")\n";
            return 13;
        }


        /* LD (nn), A

            [ 0 | 0 | 1 | 1 | 0 | 0 | 1 | 0 ]
            [ <   -   -   n   -   -   -   > ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b00110010:
        {
            incrementPC(3);
            uint16_t addr = pairBytes(byte[2], byte[1]);
            mapper_write(addr, reg[A]);
            std::clog << "LD (" << std::hex << (int)addr << "), A\n";
            return 13;
        }


        /* A Opcodes 

            [ 1 | 1 | 1 | 0 | 1 | 1 | 0 | 1 ]
        */
        case 0b11101101:
        {
            switch(byte[1]) {

                /* LD A, I
                
                    [ 0 | 1 | 0 | 1 | 0 | 1 | 1 | 1 ]
                */
                case 0b01010111:
                {
                    incrementPC(2);
                    reg[A] = interruptVector;
                    setFlag(Sign, interruptVector & 0b10000000);
                    setFlag(Zero, interruptVector == 0);
                    setFlag(HalfCarry, 0);
                    setFlag(ParityOverflow, IFF2);
                    setFlag(AddSubtract, 0);
                    std::clog << "LD A, I\n";
                    return 9;
                }


                /* LD I, A
                
                    [ 0 | 1 | 0 | 0 | 0 | 1 | 1 | 1 ]
                */
                case 0b01000111:
                {
                    incrementPC(2);
                    interruptVector = reg[A];        
                    std::clog << "LD I, A\n";
                    return 9;
                }


                /* LD A, R
                
                    [ 0 | 1 | 0 | 1 | 1 | 1 | 1 | 1 ]
                */
                case 0b01011111:
                {
                    incrementPC(2);
                    reg[A] = memoryRefresh;
                    setFlag(Sign, memoryRefresh & 0b10000000);
                    setFlag(Zero, memoryRefresh == 0);
                    setFlag(HalfCarry, 0);
                    setFlag(ParityOverflow, IFF2);
                    setFlag(AddSubtract, 0);
                    std::clog << "LD A, R\n";
                    return 9;
                }


                /* LD R, A
                
                    [ 0 | 1 | 0 | 0 | 1 | 1 | 1 | 1 ]
                */
                case 0b01001111:
                {
                    incrementPC(2);
                    memoryRefresh = reg[A];
                    std::clog << "LD R, A\n";
                    return 9;
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

                /* LD r, (IX+d)
                
                    [ 0 | 1 | <  r  > | 1 | 1 | 0 ]
                    [ <   -   -  d  -   -   -   - ]
                */
                case 0b01111110: case 0b01000110: case 0b01001110: case 0b01010110: case 0b01011110: case 0b01100110: case 0b01101110:
                {
                    incrementPC(3);
                    uint8_t rrr = (byte[1] & 0b00111000) >> 3;
                    uint16_t addr = index + (int8_t)byte[2];
                    write_rrrSymbol(rrr, mapper_read(addr));
                    std::clog << "LD " << name_rrrSymbol(rrr) << ", (IX+d)\n";
                    return 19;
                }


                /* LD (IX+d), r
                
                    [ 0 | 1 | 1 | 1 | 0 | <  r  > ]
                    [ <   -   -  d  -   -   -   - ]
                */
                case 0b01110111: case 0b01110000: case 0b01110001: case 0b01110010: case 0b01110011: case 0b01110100: case 0b01110101: 
                {
                    incrementPC(3);
                    uint8_t rrr = byte[1] & 0b00000111;
                    uint16_t addr = index + (int8_t)byte[2];
                    mapper_write(addr, read_rrrSymbol(rrr));
                    std::clog << "LD (IX+d), " << name_rrrSymbol(rrr) << " \n";
                    return 19;
                }


                /* LD (IX+d), n
                
                    [ 0 | 0 | 1 | 1 | 0 | 1 | 1 | 0 ]
                    [ <   -   -   d   -   -   -   > ]
                    [ <   -   -   n   -   -   -   > ]
                */
                case 0b00110110: 
                {
                    incrementPC(4);
                    uint16_t addr = index + (int8_t)byte[2];
                    mapper_write(addr, byte[3]);
                    std::clog << "LD (IX+d), " << std::hex << (int)byte[3] << "\n";
                    return 19;
                }


                /* LD IXl, n (Undocumented, http://www.z80.info/z80undoc.htm)

                    [ 0 | 0 | 1 | 0 | 1 | 1 | 1 | 0 ]
                    [ <   -   -   n   -   -   -   > ]
                */
                case 0b00101110:
                {
                    incrementPC(3);
                    index &= 0xFF00;
                    index |= byte[2];
                    std::clog << "LD IXl, n\n";
                    return 13;
                }


                /* LD IXh, n (Undocumented, http://www.z80.info/z80undoc.htm)

                    [ 0 | 0 | 1 | 0 | 0 | 1 | 1 | 0 ]
                    [ <   -   -   n   -   -   -   > ]
                */
                case 0b00100110:
                {
                    incrementPC(3);
                    index &= 0x00FF;
                    index |= byte[2] << 8;
                    std::clog << "LD IXh, n\n";
                    return 13;
                }


                /* LD q, IXl (Undocumented, http://www.z80.info/z80undoc.htm)

                    [ 0 | 1 | <   q   > | 1 | 0 | 1 ]
                */
                case 0b01000101: case 0b01001101: case 0b01010101: case 0b01011101: case 0b01100101: case 0b01101101: case 0b01111101:
                {
                    incrementPC(2);
                    uint8_t q = (byte[1] & 0b00111000) >> 3;
                    uint8_t data = index;

                    switch(q) {
                        case 7: reg[A] = data; break;
                        case 0: reg[B] = data; break;
                        case 1: reg[C] = data; break;
                        case 2: reg[D] = data; break;
                        case 3: reg[E] = data; break;
                        case 4: index &= 0x00FF; index |= data << 8; break;
                        case 5: index &= 0xFF00; index |= data; break;
                    }
                    std::clog << "LD r, IXl\n";
                    return 10;
                }


                /* LD q, IXh (Undocumented, http://www.z80.info/z80undoc.htm)

                    [ 0 | 1 | <   q   > | 1 | 0 | 0 ]
                */
                case 0b01000100: case 0b01001100: case 0b01010100: case 0b01011100: case 0b01100100: case 0b01101100: case 0b01111100:
                {
                    incrementPC(2);
                    uint8_t q = (byte[1] & 0b00111000) >> 3;
                    uint8_t data = index >> 8;

                    switch(q) {
                        case 7: reg[A] = data; break;
                        case 0: reg[B] = data; break;
                        case 1: reg[C] = data; break;
                        case 2: reg[D] = data; break;
                        case 3: reg[E] = data; break;
                        case 4: index &= 0x00FF; index |= data << 8; break;
                        case 5: index &= 0xFF00; index |= data; break;
                    }
                    std::clog << "LD r, IXl\n";
                    return 10;
                }
            }
            break;
        }

    }
    return 0;
}