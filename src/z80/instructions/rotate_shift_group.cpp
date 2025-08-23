#include "../z80.h"
#include <bitset>
#include <iostream>

int z80::processRotateShiftGroup() {

    uint8 byte[4] {
        mapper_read(programCounter),
        mapper_read(programCounter+1),
        mapper_read(programCounter+2),
        mapper_read(programCounter+3)
    };

    switch(byte[0]) {

        /* RLC A

            [ 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 ]
        */
        case 0b00000111:
        {
            incrementPC(1);
            RLC(reg[A], true);
            std::clog << "RLC A\n";
            return 4;
        }


        /* RL A

            [ 0 | 0 | 0 | 1 | 0 | 1 | 1 | 1 ]
        */
        case 0b00010111:
        {
            incrementPC(1);
            RL(reg[A], true);
            std::clog << "RL A\n";
            return 4;
        }


        /* RRC A

            [ 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 ]
        */
        case 0b00001111:
        {
            incrementPC(1);
            RRC(reg[A], true);
            std::clog << "RRC A\n";
            return 4;
        }


        /* RR A

            [ 0 | 0 | 0 | 1 | 1 | 1 | 1 | 1 ]
        */
        case 0b00011111:
        {
            incrementPC(1);
            RR(reg[A], true);
            std::clog << "RR A\n";
            return 4;
        }


        /* Load Opcodes
        
            [ 1 | 1 | 1 | 0 | 1 | 1 | 0 | 1 ]
        */
        case 0b11101101:
        {

            switch(byte[1]) {

                /* RLD
                
                    [ 0 | 1 | 1 | 0 | 1 | 1 | 1 | 1 ]
                */
                case 0b01101111:
                {
                    incrementPC(2);
                    uint16 addr = pairBytes(reg[H], reg[L]);
                    uint8 data = mapper_read(addr);

                    uint8 a = reg[A] & 0b00001111;
                    uint8 b = data & 0b11110000;
                    uint8 c = data & 0b00001111;

                    reg[A] &= 0b11110000;
                    reg[A] |= b >> 4;

                    data = 0;
                    data |= c << 4;
                    data |= a;

                    mapper_write(addr, data);

                    setFlag(Sign, reg[A] & 0b10000000);
                    setFlag(Zero, reg[A] == 0);
                    setFlag(HalfCarry, 0);
                    setFlag(ParityOverflow, std::bitset<8>(reg[A]).count() % 2 == 0);
                    setFlag(AddSubtract, 0);

                    std::clog << "RLD\n";
                    return 18;
                }


                /* RRD
                
                    [ 0 | 1 | 1 | 0 | 0 | 1 | 1 | 1 ]
                */
                case 0b01100111:
                {
                    incrementPC(2);
                    uint16 addr = pairBytes(reg[H], reg[L]);
                    uint8 data = mapper_read(addr);

                    uint8 a = reg[A] & 0b00001111;
                    uint8 b = data & 0b11110000;
                    uint8 c = data & 0b00001111;

                    reg[A] &= 0b11110000;
                    reg[A] |= c;

                    data = 0;
                    data |= a << 4;
                    data |= b >> 4;

                    mapper_write(addr, data);

                    setFlag(Sign, reg[A] & 0b10000000);
                    setFlag(Zero, reg[A] == 0);
                    setFlag(HalfCarry, 0);
                    setFlag(ParityOverflow, std::bitset<8>(reg[A]).count() % 2 == 0);
                    setFlag(AddSubtract, 0);

                    std::clog << "RRD\n";
                    return 18;
                }
            }
            break;
        }


        /* Shift Opcodes
        
            [ 1 | 1 | 0 | 0 | 1 | 0 | 1 | 1 ]
        */
        case 0b11001011:
        {

            switch(byte[1]) {

                /* RLC r

                    [ 0 | 0 | 0 | 0 | 0 | <   r   > ]
                */
                case 0b00000111: case 0b00000000: case 0b00000001: case 0b00000010: case 0b00000011: case 0b00000100: case 0b00000101:
                {
                    incrementPC(2);
                    uint8 rrr = byte[1] & 0b00000111;
                    uint8 data = read_rrrSymbol(rrr);
                    RLC(data);
                    write_rrrSymbol(rrr, data);
                    std::clog << "RLC " << name_rrrSymbol(rrr) << "\n";
                    return 8;
                }


                /* SLL r (Undocumented, https://jnz.dk/z80/sll_r.html)
                
                    [ 0 | 0 | 1 | 1 | 0 | <   r   > ]
                */
                case 0b00110111: case 0b00110000: case 0b00110001: case 0b00110010: case 0b00110011: case 0b00110100: case 0b00110101:
                {
                    incrementPC(2);
                    uint8 rrr = byte[1] & 0b00000111;
                    uint8 data = read_rrrSymbol(rrr);
                    SLL(data);
                    write_rrrSymbol(rrr, data);
                    std::clog << "SLL " << name_rrrSymbol(rrr) << "\n";
                    return 8;
                }


                /* RL r

                    [ 0 | 0 | 0 | 1 | 0 | <   r   > ]
                */
                case 0b00010111: case 0b00010000: case 0b00010001: case 0b00010010: case 0b00010011: case 0b00010100: case 0b00010101:
                {
                    incrementPC(2);
                    uint8 rrr = byte[1] & 0b00000111;
                    uint8 data = read_rrrSymbol(rrr);
                    RL(data);
                    write_rrrSymbol(rrr, data);
                    std::clog << "RL " << name_rrrSymbol(rrr) << "\n";
                    return 8;
                }


                /* RRC r

                    [ 0 | 0 | 0 | 0 | 1 | <   r   > ]
                */
                case 0b00001111: case 0b00001000: case 0b00001001: case 0b00001010: case 0b00001011: case 0b00001100: case 0b00001101:
                {
                    incrementPC(2);
                    uint8 rrr = byte[1] & 0b00000111;
                    uint8 data = read_rrrSymbol(rrr);
                    RRC(data);
                    write_rrrSymbol(rrr, data);
                    std::clog << "RRC " << name_rrrSymbol(rrr) << "\n";
                    return 8;
                }


                /* RR r

                    [ 0 | 0 | 0 | 1 | 1 | <   r   > ]
                */
                case 0b00011111: case 0b00011000: case 0b00011001: case 0b00011010: case 0b00011011: case 0b00011100: case 0b00011101:
                {
                    incrementPC(2);
                    uint8 rrr = byte[1] & 0b00000111;
                    uint8 data = read_rrrSymbol(rrr);
                    RR(data);
                    write_rrrSymbol(rrr, data);
                    std::clog << "RR " << name_rrrSymbol(rrr) << "\n";
                    return 8;
                }


                /* SLA r

                    [ 0 | 0 | 1 | 0 | 0 | <   r   > ]
                */
                case 0b00100111: case 0b00100000: case 0b00100001: case 0b00100010: case 0b00100011: case 0b00100100: case 0b00100101:
                {
                    incrementPC(2);
                    uint8 rrr = byte[1] & 0b00000111;
                    uint8 data = read_rrrSymbol(rrr);
                    SLA(data);
                    write_rrrSymbol(rrr, data);
                    std::clog << "SLA " << name_rrrSymbol(rrr) << "\n";
                    return 8;
                }


                /* SRA r

                    [ 0 | 0 | 1 | 0 | 1 | <   r   > ]
                */
                case 0b00101111: case 0b00101000: case 0b00101001: case 0b00101010: case 0b00101011: case 0b00101100: case 0b00101101:
                {
                    incrementPC(2);
                    uint8 rrr = byte[1] & 0b00000111;
                    uint8 data = read_rrrSymbol(rrr);
                    SRA(data);
                    write_rrrSymbol(rrr, data);
                    std::clog << "SRA " << name_rrrSymbol(rrr) << "\n";
                    return 8;
                }


                /* SRL r

                    [ 0 | 0 | 1 | 1 | 1 | <   r   > ]
                */
                case 0b00111111: case 0b00111000: case 0b00111001: case 0b00111010: case 0b00111011: case 0b00111100: case 0b00111101:
                {
                    incrementPC(2);
                    uint8 rrr = byte[1] & 0b00000111;
                    uint8 data = read_rrrSymbol(rrr);
                    SRL(data);
                    write_rrrSymbol(rrr, data);
                    std::clog << "SRL " << name_rrrSymbol(rrr) << "\n";
                    return 8;
                }


                /* RLC (HL)
                
                    [ 0 | 0 | 0 | 0 | 0 | 1 | 1 | 0 ]
                */
                case 0b00000110:
                {
                    incrementPC(2);
                    uint16 addr = pairBytes(reg[H], reg[L]);
                    uint8 data = mapper_read(addr);
                    RLC(data);
                    mapper_write(addr, data);
                    std::clog << "RLC (HL)\n";
                    return 15;
                }


                /* SLL (HL) (Undocumented, https://jnz.dk/z80/sll_hlp.html)
                
                    [ 0 | 0 | 1 | 1 | 0 | 1 | 1 | 0 ]
                */
                case 0b00110110: 
                {
                    incrementPC(2);
                    uint16 addr = pairBytes(reg[H], reg[L]);
                    uint8 data = mapper_read(addr);
                    SLL(data);
                    mapper_write(addr, data);
                    std::clog << "SLL (HL)\n";
                    return 15;
                }


                /* RL (HL)
                
                    [ 0 | 0 | 0 | 1 | 0 | 1 | 1 | 0 ]
                */
                case 0b00010110:
                {
                    incrementPC(2);
                    uint16 addr = pairBytes(reg[H], reg[L]);
                    uint8 data = mapper_read(addr);
                    RL(data);
                    mapper_write(addr, data);
                    std::clog << "RL (HL)\n";
                    return 15;
                }


                /* RRC (HL)
                
                    [ 0 | 0 | 0 | 0 | 1 | 1 | 1 | 0 ]
                */
                case 0b00001110:
                {
                    incrementPC(2);
                    uint16 addr = pairBytes(reg[H], reg[L]);
                    uint8 data = mapper_read(addr);
                    RRC(data);
                    mapper_write(addr, data);
                    std::clog << "RRC (HL)\n";
                    return 15;
                }


                /* RR (HL)
                
                    [ 0 | 0 | 0 | 1 | 1 | 1 | 1 | 0 ]
                */
                case 0b00011110:
                {
                    incrementPC(2);
                    uint16 addr = pairBytes(reg[H], reg[L]);
                    uint8 data = mapper_read(addr);
                    RR(data);
                    mapper_write(addr, data);
                    std::clog << "RR (HL)\n";
                    return 15;
                }


                /* SLA (HL)
                
                    [ 0 | 0 | 1 | 0 | 0 | 1 | 1 | 0 ]
                */
                case 0b00100110:
                {
                    incrementPC(2);
                    uint16 addr = pairBytes(reg[H], reg[L]);
                    uint8 data = mapper_read(addr);
                    SLA(data);
                    mapper_write(addr, data);
                    std::clog << "SLA (HL)\n";
                    return 15;
                }


                /* SRA (HL)
                
                    [ 0 | 0 | 1 | 0 | 1 | 1 | 1 | 0 ]
                */
                case 0b00101110:
                {
                    incrementPC(2);
                    uint16 addr = pairBytes(reg[H], reg[L]);
                    uint8 data = mapper_read(addr);
                    SRA(data);
                    mapper_write(addr, data);
                    std::clog << "SRA (HL)\n";
                    return 15;
                }


                /* SRL (HL)
                
                    [ 0 | 0 | 1 | 1 | 1 | 1 | 1 | 0 ]
                */
                case 0b00111110:
                {
                    incrementPC(2);
                    uint16 addr = pairBytes(reg[H], reg[L]);
                    uint8 data = mapper_read(addr);
                    SRL(data);
                    mapper_write(addr, data);
                    std::clog << "SRL (HL)\n";
                    return 8;
                }
            }
        }

        /* IX/IY Opcodes
        
            [ 1 | 1 | x | 1 | 1 | 1 | 0 | 1 ]
            IX = 0
            IY = 1
        */
        case 0b11011101: case 0b11111101:
        {
            uint16& index = (byte[0] == 0b11011101) ? indexRegisterX : indexRegisterY;

            // [ 1 | 1 | 0 | 0 | 1 | 0 | 1 | 1 ]
            if(byte[1] == 0b11001011) {

                switch(byte[3]) {


                    /* RLC (IX+d)
                    
                        [ 0 | 0 | 0 | 0 | 0 | 1 | 1 | 0 ]
                    */
                    case 0b00000110:
                    {
                        incrementPC(4);
                        uint16 addr = index + (sint8)byte[2];
                        uint8 data = mapper_read(addr);
                        RLC(data);
                        mapper_write(addr, data);
                        std::clog << "RLC (IX+d)\n";
                        return 23;
                    }


                    /* SLL (IX+d) (Undocumented, https://jnz.dk/z80/sll_ixdp.html)
                    
                        [ 0 | 0 | 1 | 1 | 0 | 1 | 1 | 0 ]
                    */
                    case 0b00110110: 
                    {
                        incrementPC(4);
                        uint16 addr = index + (sint8)byte[2];
                        uint8 data = mapper_read(addr);
                        SLL(data);
                        mapper_write(addr, data);
                        std::clog << "SLL (IX+d)\n";
                        return 23;
                    }


                    /* RL (IX+d)
                    
                        [ 0 | 0 | 0 | 1 | 0 | 1 | 1 | 0 ]
                    */
                    case 0b00010110:
                    {
                        incrementPC(4);
                        uint16 addr = index + (sint8)byte[2];
                        uint8 data = mapper_read(addr);
                        RL(data);
                        mapper_write(addr, data);
                        std::clog << "RL (IX+d)\n";
                        return 23;
                    }


                    /* RRC (IX+d)
                    
                        [ 0 | 0 | 0 | 0 | 1 | 1 | 1 | 0 ]
                    */
                    case 0b00001110:
                    {
                        incrementPC(4);
                        uint16 addr = index + (sint8)byte[2];
                        uint8 data = mapper_read(addr);
                        RRC(data);
                        mapper_write(addr, data);
                        std::clog << "RRC (IX+d)\n";
                        return 23;
                    }


                    /* RR (IX+d)
                    
                        [ 0 | 0 | 0 | 1 | 1 | 1 | 1 | 0 ]
                    */
                    case 0b00011110:
                    {
                        incrementPC(4);
                        uint16 addr = index + (sint8)byte[2];
                        uint8 data = mapper_read(addr);
                        RR(data);
                        mapper_write(addr, data);
                        std::clog << "RR (IX+d)\n";
                        return 23;
                    }


                    /* SLA (IX+d)
                    
                        [ 0 | 0 | 1 | 0 | 0 | 1 | 1 | 0 ]
                    */
                    case 0b00100110:
                    {
                        incrementPC(4);
                        uint16 addr = index + (sint8)byte[2];
                        uint8 data = mapper_read(addr);
                        SLA(data);
                        mapper_write(addr, data);
                        std::clog << "SLA (IX+d)\n";
                        return 23;
                    }


                    /* SRA (IX+d)
                    
                        [ 0 | 0 | 1 | 0 | 1 | 1 | 1 | 0 ]
                    */
                    case 0b00101110:
                    {
                        incrementPC(4);
                        uint16 addr = index + (sint8)byte[2];
                        uint8 data = mapper_read(addr);
                        SRA(data);
                        mapper_write(addr, data);
                        std::clog << "SRA (IX+d)\n";
                        return 23;
                    }


                    /* SRL (IX+d)
                    
                        [ 0 | 0 | 1 | 1 | 1 | 1 | 1 | 0 ]
                    */
                    case 0b00111110:
                    {
                        incrementPC(4);
                        uint16 addr = index + (sint8)byte[2];
                        uint8 data = mapper_read(addr);
                        SRL(data);
                        mapper_write(addr, data);
                        std::clog << "SRL (IX+d)\n";
                        return 23;
                    }
                }
            }
        }

    }
    return 0;
}