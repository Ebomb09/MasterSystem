#include "../z80.h"
#include <iostream>
#include <bitset>

int z80::process8BitArithmeticGroup() {

    uint8 byte[4] {
        mapper_read(programCounter),
        mapper_read(programCounter+1),
        mapper_read(programCounter+2),
        mapper_read(programCounter+3)
    };

    switch(byte[0]) {

        /* ADD A, r
        
            [ 1 | 0 | 0 | 0 | 0 | <   r   > ]
        */
        case 0b10000111: case 0b10000000: case 0b10000001: case 0b10000010: case 0b10000011: case 0b10000100: case 0b10000101:
        {
            incrementPC(1);
            uint8 rrr = byte[0] & 0b00000111;
            ADD(reg[A], read_rrrSymbol(rrr));
            std::clog << "ADD A, " << name_rrrSymbol(rrr) << "\n";
            return 4;
        }


        /* SUB A, r
        
            [ 1 | 0 | 0 | 1 | 0 | <   r   > ]
        */
        case 0b10010111: case 0b10010000: case 0b10010001: case 0b10010010: case 0b10010011: case 0b10010100: case 0b10010101:
        {
            incrementPC(1);
            uint8 rrr = byte[0] & 0b00000111;
            SUB(reg[A], read_rrrSymbol(rrr));
            std::clog << "SUB A, " << name_rrrSymbol(rrr) << "\n";
            return 4;
        }


        /* AND A, r
        
            [ 1 | 0 | 1 | 0 | 0 | <   r   > ]
        */
        case 0b10100111: case 0b10100000: case 0b10100001: case 0b10100010: case 0b10100011: case 0b10100100: case 0b10100101:
        {
            incrementPC(1);
            uint8 rrr = byte[0] & 0b00000111;
            AND(reg[A], read_rrrSymbol(rrr));
            std::clog << "AND A, " << name_rrrSymbol(rrr) << "\n";
            return 4;
        }


        /* OR A, r
        
            [ 1 | 0 | 1 | 1 | 0 | <   r   > ]
        */
        case 0b10110111: case 0b10110000: case 0b10110001: case 0b10110010: case 0b10110011: case 0b10110100: case 0b10110101:
        {
            incrementPC(1);
            uint8 rrr = byte[0] & 0b00000111;
            OR(reg[A], read_rrrSymbol(rrr));
            std::clog << "OR A, " << name_rrrSymbol(rrr) << "\n";
            return 4;
        }


        /* XOR A, r
        
            [ 1 | 0 | 1 | 0 | 1 | <   r   > ]
        */
        case 0b10101111: case 0b10101000: case 0b10101001: case 0b10101010: case 0b10101011: case 0b10101100: case 0b10101101:
        {
            incrementPC(1);
            uint8 rrr = byte[0] & 0b00000111;
            XOR(reg[A], read_rrrSymbol(rrr));
            std::clog << "XOR A, " << name_rrrSymbol(rrr) << "\n";
            return 4;
        }


        /* CP A, r
        
            [ 1 | 0 | 1 | 1 | 1 | <   r   > ]
        */
        case 0b10111111: case 0b10111000: case 0b10111001: case 0b10111010: case 0b10111011: case 0b10111100: case 0b10111101:
        {
            incrementPC(1);
            uint8 rrr = byte[0] & 0b00000111;
            CP(reg[A], read_rrrSymbol(rrr));
            std::clog << "CP A, " << name_rrrSymbol(rrr) << "\n";
            return 4;
        }


        /* INC r
        
            [ 0 | 0 | <   r   > | 1 | 0 | 0 ]
        */
        case 0b00111100: case 0b00000100: case 0b00001100: case 0b00010100: case 0b00011100: case 0b00100100: case 0b00101100:
        {
            incrementPC(1);
            uint8 rrr = (byte[0] & 0b00111000) >> 3;
            uint8 data = read_rrrSymbol(rrr);
            INC(data);
            write_rrrSymbol(rrr, data);
            std::clog << "INC " << name_rrrSymbol(rrr) << "\n";
            return 4;
        }


        /* DEC r
        
            [ 0 | 0 | <   r   > | 1 | 0 | 1 ]
        */
        case 0b00111101: case 0b00000101: case 0b00001101: case 0b00010101: case 0b00011101: case 0b00100101: case 0b00101101:
        {
            incrementPC(1);
            uint8 rrr = (byte[0] & 0b00111000) >> 3;
            uint8 data = read_rrrSymbol(rrr);
            DEC(data);
            write_rrrSymbol(rrr, data);
            std::clog << "DEC " << name_rrrSymbol(rrr) << "\n";
            return 4;
        }


        /* ADD A, n
        
            [ 1 | 1 | 0 | 0 | 0 | 1 | 1 | 0 ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b11000110:
        {
            incrementPC(2);
            ADD(reg[A], byte[1]);
            std::clog << "ADD A, " << std::hex << (int)byte[1] << "\n";
            return 7;
        }


        /* SUB A, n
        
            [ 1 | 1 | 0 | 1 | 0 | 1 | 1 | 0 ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b11010110:
        {
            incrementPC(2);
            SUB(reg[A], byte[1]);
            std::clog << "SUB A, " << std::hex << (int)byte[1] << "\n";
            return 7;
        }


        /* AND A, n
        
            [ 1 | 1 | 1 | 0 | 0 | 1 | 1 | 0 ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b11100110:
        {
            incrementPC(2);
            AND(reg[A], byte[1]);
            std::clog << "AND A, " << std::hex << (int)byte[1] << "\n";
            return 7;
        }


        /* OR A, n
        
            [ 1 | 1 | 1 | 1 | 0 | 1 | 1 | 0 ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b11110110:
        {
            incrementPC(2);
            OR(reg[A], byte[1]);
            std::clog << "OR A, " << std::hex << (int)byte[1] << "\n";
            return 7;
        }


        /* XOR A, n
        
            [ 1 | 1 | 1 | 0 | 1 | 1 | 1 | 0 ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b11101110:
        {
            incrementPC(2);
            XOR(reg[A], byte[1]);
            std::clog << "OR A, " << std::hex << (int)byte[1] << "\n";
            return 7;
        }


        /* CP A, n
        
            [ 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b11111110:
        {
            incrementPC(2);
            CP(reg[A], byte[1]);
            std::clog << "CP A, " << std::hex << (int)byte[1] << "\n";
            return 7;
        }


        /* ADD A, (HL)
        
            [ 1 | 0 | 0 | 0 | 0 | 1 | 1 | 0 ]
        */
        case 0b10000110:
        {
            incrementPC(1);
            uint8 data = mapper_read(pairBytes(reg[H], reg[L]));
            ADD(reg[A], data);
            std::clog << "ADD A, (HL)\n";
            return 7;
        }


        /* SUB A, (HL)
        
            [ 1 | 0 | 0 | 1 | 0 | 1 | 1 | 0 ]
        */
        case 0b10010110:
        {
            incrementPC(1);
            uint8 data = mapper_read(pairBytes(reg[H], reg[L]));
            SUB(reg[A], data);
            std::clog << "SUB A, (HL)\n";
            return 7;
        }


        /* AND A, (HL)
        
            [ 1 | 0 | 1 | 0 | 0 | 1 | 1 | 0 ]
        */
        case 0b10100110:
        {
            incrementPC(1);
            uint8 data = mapper_read(pairBytes(reg[H], reg[L]));
            AND(reg[A], data);
            std::clog << "AND A, (HL)\n";
            return 7;
        }


        /* OR A, (HL)
        
            [ 1 | 0 | 1 | 1 | 0 | 1 | 1 | 0 ]
        */
        case 0b10110110:
        {
            incrementPC(1);
            uint8 data = mapper_read(pairBytes(reg[H], reg[L]));
            OR(reg[A], data);
            std::clog << "OR A, (HL)\n";
            return 7;
        }


        /* XOR A, (HL)
        
            [ 1 | 0 | 1 | 0 | 1 | 1 | 1 | 0 ]
        */
        case 0b10101110:
        {
            incrementPC(1);
            uint8 data = mapper_read(pairBytes(reg[H], reg[L]));
            XOR(reg[A], data);
            std::clog << "XOR A, (HL)\n";
            return 7;
        }


        /* CP A, (HL)
        
            [ 1 | 0 | 1 | 1 | 1 | 1 | 1 | 0 ]
        */
        case 0b10111110:
        {
            incrementPC(1);
            uint8 data = mapper_read(pairBytes(reg[H], reg[L]));
            CP(reg[A], data);
            std::clog << "CP A, (HL)\n";
            return 7;
        }


        /* INC (HL)
        
            [ 0 | 0 | 1 | 1 | 0 | 1 | 0 | 0 ]
        */
        case 0b00110100:
        {
            incrementPC(1);
            uint8 data = mapper_read(pairBytes(reg[H], reg[L]));
            INC(data);
            mapper_write(pairBytes(reg[H], reg[L]), data);
            std::clog << "INC (HL)\n";
            return 11;
        }


        /* DEC (HL)
        
            [ 0 | 0 | 1 | 1 | 0 | 1 | 0 | 1 ]
        */
        case 0b00110101:
        {
            incrementPC(1);
            uint8 data = mapper_read(pairBytes(reg[H], reg[L]));
            DEC(data);
            mapper_write(pairBytes(reg[H], reg[L]), data);
            std::clog << "DEC (HL)\n";
            return 11;
        }


        /* ADC A, r
        
            [ 1 | 0 | 0 | 0 | 1 | <   r   > ]
        */
        case 0b10001111:  case 0b10001000:  case 0b10001001:  case 0b10001010:  case 0b10001011:  case 0b10001100:  case 0b10001101:
        {
            incrementPC(1);
            uint8 rrr = byte[0] & 0b00000111;
            ADC(reg[A], read_rrrSymbol(rrr));
            std::clog << "ADC A, " << name_rrrSymbol(rrr) << "\n";
            return 4;
        }


        /* SBC A, r
        
            [ 1 | 0 | 0 | 1 | 1 | <   r   > ]
        */
        case 0b10011111:  case 0b10011000:  case 0b10011001:  case 0b10011010:  case 0b10011011:  case 0b10011100:  case 0b10011101:
        {
            incrementPC(1);
            uint8 rrr = byte[0] & 0b00000111;
            SBC(reg[A], read_rrrSymbol(rrr));
            std::clog << "SBC A, " << name_rrrSymbol(rrr) << "\n";
            return 4;
        }


        /* ADC A, n
        
            [ 1 | 1 | 0 | 0 | 1 | 1 | 1 | 0 ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b11001110:
        {
            incrementPC(2);
            ADC(reg[A], byte[1]);
            std::clog << "ADC A, " << std::hex << (int)byte[1] << "\n";
            return 7;
        }


        /* SBC A, n
        
            [ 1 | 1 | 0 | 1 | 1 | 1 | 1 | 0 ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b11011110:
        {
            incrementPC(2);
            SBC(reg[A], byte[1]);
            std::clog << "SBC A, " << std::hex << (int)byte[1] << "\n";
            return 7;
        }


        /* ADC A, (HL)
        
            [ 1 | 0 | 0 | 0 | 1 | 1 | 1 | 0 ]
        */
        case 0b10001110:
        {
            incrementPC(1);
            uint16 addr = pairBytes(reg[H], reg[L]);
            ADC(reg[A], mapper_read(addr));
            std::clog << "ADC A, (HL)\n";
            return 7;
        }


        /* SBC A, (HL)
        
            [ 1 | 0 | 0 | 1 | 1 | 1 | 1 | 0 ]
        */
        case 0b10011110:
        {
            incrementPC(1);
            uint16 addr = pairBytes(reg[H], reg[L]);
            SBC(reg[A], mapper_read(addr));
            std::clog << "SBC A, (HL)\n";
            return 7;
        }


        /* IX / IY Opcodes
        
            [ 1 | 1 | x | 1 | 1 | 1 | 0 | 1 ]
            IX = 0
            IY = 1
        */
        case 0b11011101: case 0b11111101:
        {
            uint16& index = (byte[0] == 0b11011101) ? indexRegisterX : indexRegisterY;

            switch(byte[1]) {

                /* ADD A, (IX+d)
                
                    [ 1 | 0 | 0 | 0 | 0 | 1 | 1 | 0 ]
                    [ <   -   -   d   -   -   -   > ]
                */
                case 0b10000110:
                {
                    incrementPC(3);
                    uint16 addr = index + (sint8)byte[2];
                    ADD(reg[A], mapper_read(addr));
                    std::clog << "ADD A, (IX+" << std::hex << (sint8)byte[2] << ")\n";
                    return 19;
                }


                /* SUB A, (IX+d)
                
                    [ 1 | 0 | 0 | 1 | 0 | 1 | 1 | 0 ]
                    [ <   -   -   d   -   -   -   > ]
                */
                case 0b10010110:
                {
                    incrementPC(3);
                    uint16 addr = index + (sint8)byte[2];
                    SUB(reg[A], mapper_read(addr));
                    std::clog << "SUB A, (IX+" << std::hex << (sint8)byte[2] << ")\n";
                    return 19;
                }


                /* ADC A, (IX+d)
                
                    [ 1 | 0 | 0 | 0 | 1 | 1 | 1 | 0 ]
                    [ <   -   -   d   -   -   -   > ]
                */
                case 0b10001110:
                {
                    incrementPC(3);
                    uint16 addr = index + (sint8)byte[2];
                    ADC(reg[A], mapper_read(addr));
                    std::clog << "ADC A, (IX+" << std::hex << (sint8)byte[2] << ")\n";
                    return 19;
                }


                /* SBC A, (IX+d)
                
                    [ 1 | 0 | 0 | 1 | 1 | 1 | 1 | 0 ]
                    [ <   -   -   d   -   -   -   > ]
                */
                case 0b10011110:
                {
                    incrementPC(3);
                    uint16 addr = index + (sint8)byte[2];
                    SBC(reg[A], mapper_read(addr));
                    std::clog << "SBC A, (IX+" << std::hex << (sint8)byte[2] << ")\n";
                    return 19;
                }


                /* AND A, (IX+d)
                
                    [ 1 | 0 | 1 | 0 | 0 | 1 | 1 | 0 ]
                    [ <   -   -   d   -   -   -   > ]
                */
                case 0b10100110:
                {
                    incrementPC(3);
                    uint16 addr = index + (sint8)byte[2];
                    AND(reg[A], mapper_read(addr));
                    std::clog << "AND A, (IX+" << std::hex << (sint8)byte[2] << ")\n";
                    return 19;
                }


                /* OR A, (IX+d)
                
                    [ 1 | 0 | 1 | 1 | 0 | 1 | 1 | 0 ]
                    [ <   -   -   d   -   -   -   > ]
                */
                case 0b10110110:
                {
                    incrementPC(3);
                    uint16 addr = index + (sint8)byte[2];
                    OR(reg[A], mapper_read(addr));
                    std::clog << "OR A, (IX+" << std::hex << (sint8)byte[2] << ")\n";
                    return 19;
                }


                /* XOR A, (IX+d)
                
                    [ 1 | 0 | 1 | 0 | 1 | 1 | 1 | 0 ]
                    [ <   -   -   d   -   -   -   > ]
                */
                case 0b10101110:
                {
                    incrementPC(3);
                    uint16 addr = index + (sint8)byte[2];
                    XOR(reg[A], mapper_read(addr));
                    std::clog << "XOR A, (IX+" << std::hex << (sint8)byte[2] << ")\n";
                    return 19;
                }


                /* CP A, (IX+d)
                
                    [ 1 | 0 | 1 | 1 | 1 | 1 | 1 | 0 ]
                    [ <   -   -   d   -   -   -   > ]
                */
                case 0b10111110:
                {
                    incrementPC(3);
                    uint16 addr = index + (sint8)byte[2];
                    CP(reg[A], mapper_read(addr));
                    std::clog << "CP A, (IX+" << std::hex << (sint8)byte[2] << ")\n";
                    return 19;
                }


                /* INC (IX+d)
                
                    [ 0 | 0 | 1 | 1 | 0 | 1 | 0 | 0 ]
                    [ <   -   -   d   -   -   -   > ]
                */
                case 0b00110100:
                {
                    incrementPC(3);
                    uint16 addr = index + (sint8)byte[2];
                    uint8 data = mapper_read(addr);
                    INC(data);
                    mapper_write(addr, data);
                    std::clog << "INC (IX+" << std::hex << (sint8)byte[2] << ")\n";
                    return 23;
                }


                /* DEC (IX+d)
                
                    [ 0 | 0 | 1 | 1 | 0 | 1 | 0 | 1 ]
                    [ <   -   -   d   -   -   -   > ]
                */
                case 0b00110101:
                {
                    incrementPC(3);
                    uint16 addr = index + (sint8)byte[2];
                    uint8 data = mapper_read(addr);
                    DEC(data);
                    mapper_write(addr, data);
                    std::clog << "INC (IX+" << std::hex << (sint8)byte[2] << ")\n";
                    return 23;
                }


                /* INC IXh/IXl (Undocumented, http://www.z80.info/z80undoc.htm)
                
                    [ 0 | 0 | 1 | 0 | b | 1 | 0 | 0 ]
                */
                case 0b00100100: case 0b00101100:
                {
                    incrementPC(2);

                    uint8 data = index;
                    if(byte[1] & 0b00001000)    data = index;
                    else                        data = index >> 8; 

                    INC(data);

                    if(byte[1] & 0b00001000)    {index &= 0xFF00; index |= data; }
                    else                        {index &= 0x00FF; index |= data << 8; }

                    std::clog << "INC IXh\n";
                    return 10;
                }


                /* DEC IXh/IXl (Undocumented, http://www.z80.info/z80undoc.htm)
                
                    [ 0 | 0 | 1 | 0 | b | 1 | 0 | 1 ]
                */
                case 0b00100101: case 0b00101101:
                {
                    incrementPC(2);

                    uint8 data = index;
                    if(byte[1] & 0b00001000)    data = index;
                    else                        data = index >> 8; 

                    DEC(data);

                    if(byte[1] & 0b00001000)    {index &= 0xFF00; index |= data; }
                    else                        {index &= 0x00FF; index |= data << 8; }

                    std::clog << "DEC IXh\n";
                    return 10;
                }


                /* ADD A, IXl/IXh (Undocumented, http://www.z80.info/z80undoc.htm)
                
                    [ 1 | 0 | 0 | 0 | 0 | 1 | 0 | b ]
                */
                case 0b10000100: case 0b10000101:
                {
                    incrementPC(2);

                    uint8 data;
                    if(byte[1] & 0b00000001) data = index;
                    else                     data = index >> 8;

                    ADD(reg[A], data);
                    std::clog << "ADD A, IX\n";
                    return 10;
                }


                /* ADC A, IXl/IXh (Undocumented, http://www.z80.info/z80undoc.htm)
                
                    [ 1 | 0 | 0 | 0 | 1 | 1 | 0 | b ]
                */
                case 0b10001100: case 0b10001101:
                {
                    incrementPC(2);

                    uint8 data;
                    if(byte[1] & 0b00000001) data = index;
                    else                     data = index >> 8;
                    
                    ADC(reg[A], data);
                    std::clog << "ADC A, IX\n";
                    return 10;
                }


                /* SUB A, IXl/IXh (Undocumented, http://www.z80.info/z80undoc.htm)
                
                    [ 1 | 0 | 0 | 1 | 0 | 1 | 0 | b ]
                */
                case 0b10010100: case 0b10010101:
                {
                    incrementPC(2);

                    uint8 data;
                    if(byte[1] & 0b00000001) data = index;
                    else                     data = index >> 8;
                    
                    SUB(reg[A], data);
                    std::clog << "SUB A, IX\n";
                    return 10;
                }


                /* SBC A, IXl/IXh (Undocumented, http://www.z80.info/z80undoc.htm)
                
                    [ 1 | 0 | 0 | 1 | 1 | 1 | 0 | b ]
                */
                case 0b10011100: case 0b10011101:
                {
                    incrementPC(2);

                    uint8 data;
                    if(byte[1] & 0b00000001) data = index;
                    else                     data = index >> 8;
                    
                    SBC(reg[A], data);
                    std::clog << "SBC A, IX\n";
                    return 10;
                }


                /* AND A, IXl/IXh (Undocumented, http://www.z80.info/z80undoc.htm)
                
                    [ 1 | 0 | 1 | 0 | 0 | 1 | 0 | b ]
                */
                case 0b10100100: case 0b10100101:
                {
                    incrementPC(2);

                    uint8 data;
                    if(byte[1] & 0b00000001) data = index;
                    else                     data = index >> 8;
                    
                    AND(reg[A], data);
                    std::clog << "AND A, IX\n";
                    return 10;
                }


                /* XOR A, IXl/IXh (Undocumented, http://www.z80.info/z80undoc.htm)
                
                    [ 1 | 0 | 1 | 0 | 1 | 1 | 0 | b ]
                */
                case 0b10101100: case 0b10101101:
                {
                    incrementPC(2);

                    uint8 data;
                    if(byte[1] & 0b00000001) data = index;
                    else                     data = index >> 8;
                    
                    XOR(reg[A], data);
                    std::clog << "XOR A, IX\n";
                    return 10;
                }


                /* OR A, IXl/IXh (Undocumented, http://www.z80.info/z80undoc.htm)
                
                    [ 1 | 0 | 1 | 1 | 0 | 1 | 0 | b ]
                */
                case 0b10110100: case 0b10110101:
                {
                    incrementPC(2);

                    uint8 data;
                    if(byte[1] & 0b00000001) data = index;
                    else                     data = index >> 8;
                    
                    OR(reg[A], data);
                    std::clog << "OR A, IX\n";
                    return 10;
                }


                /* CP A, IXl/IXh (Undocumented, http://www.z80.info/z80undoc.htm)
                
                    [ 1 | 0 | 1 | 1 | 1 | 1 | 0 | b ]
                */
                case 0b10111100: case 0b10111101:
                {
                    incrementPC(2);

                    uint8 data;
                    if(byte[1] & 0b00000001) data = index;
                    else                     data = index >> 8;
                    
                    CP(reg[A], data);
                    std::clog << "CP A, IX\n";
                    return 10;
                }
            }
            break;
        }
    }
    return 0;
}