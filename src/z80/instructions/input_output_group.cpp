#include "z80/z80.h"
#include "common/utilities.h"
#include <bitset>
#include <iostream>

int z80::processInputOutputGroup() {

    uint8_t byte[4] {
        mapper_read(programCounter),
        mapper_read(programCounter+1),
        mapper_read(programCounter+2),
        mapper_read(programCounter+3)
    };

    switch(byte[0]) {

        /* IN A, (n)
        
            [ 1 | 1 | 0 | 1 | 1 | 0 | 1 | 1 ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b11011011:
        {
            reg[A] = port_read(byte[1]);
            programCounter += 2;

            std::clog << "IN A, (" << std::hex << (int)byte[1] << ")\n";
            return 11;
        }


        /* OUT (n), A
        
            [ 1 | 1 | 0 | 1 | 0 | 0 | 1 | 1 ]
            [ <   -   -   n   -   -   -   > ]
        */
        case 0b11010011:
        {
            port_write(byte[1], reg[A]);
            programCounter += 2;

            std::clog << "OUT (" << std::hex <<(int)byte[1] << "), A\n";
            return 11;
        }


        /* IN / OUT Opcodes
        
            [ 1 | 1 | 1 | 0 | 1 | 1 | 0 | 1 ]
        */
        case 0b11101101:
        {

            switch(byte[1]) {

                /* IN r, (C)
                
                    [ 0 | 1 | <   r   > | 0 | 0 | 0 ]
                */
                case 0b01111000: case 0b01000000: case 0b01001000: case 0b01010000: case 0b01011000: case 0b01100000: case 0b01101000:
                {
                    uint8_t rrr = (byte[1] & 0b00111000) >> 3;

                    uint8_t data = port_read(reg[C]);
                    write_rrrSymbol(rrr, data);

                    setFlag(Sign, data & 0b10000000);
                    setFlag(Zero, data == 0);
                    setFlag(HalfCarry, 0);
                    setFlag(ParityOverflow, std::bitset<8>(data).count() % 2 == 0);
                    setFlag(AddSubtract, 0);

                    programCounter += 2;

                    std::clog << "IN " << name_rrrSymbol(rrr) << ", (C)\n";
                    return 12;
                }

                /* INI

                    [ 1 | 0 | 1 | 0 | 0 | 0 | 1 | 0 ]
                */
                case 0b10100010:
                {
                    uint16_t ptr = pairBytes(reg[H], reg[L]);

                    uint8_t data = port_read(reg[C]);
                    mapper_write(ptr, data);

                    // Decrement B
                    reg[B] = reg[B] - 1;

                    // Decrement HL
                    ptr = ptr + 1;
                    reg[H] = ptr >> 8;
                    reg[L] = ptr;

                    setFlag(Zero, reg[B] == 0);
                    setFlag(AddSubtract, 1);

                    programCounter += 2;

                    std::clog << "INI\n";
                    return 16;
                }


                /* INIR

                    [ 1 | 0 | 1 | 1 | 0 | 0 | 1 | 0 ]
                */
                case 0b10110010:
                {
                    uint16_t ptr = pairBytes(reg[H], reg[L]);

                    uint8_t data = port_read(reg[C]);
                    mapper_write(ptr, data);

                    // Decrement B
                    reg[B] = reg[B] - 1;

                    // Increment HL
                    ptr = ptr + 1;
                    reg[H] = ptr >> 8;
                    reg[L] = ptr;

                    setFlag(Zero, 1);
                    setFlag(AddSubtract, 1);

                    std::clog << "INIR\n";

                    if(reg[B] == 0) {
                        programCounter += 2;
                        return 16;

                    }else {
                        return 21;
                    }
                }


                /* IND

                    [ 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 ]
                */
                case 0b10101010:
                {
                    uint16_t ptr = pairBytes(reg[H], reg[L]);

                    uint8_t data = port_read(reg[C]);
                    mapper_write(ptr, data);

                    // Decrement B
                    reg[B] = reg[B] - 1;

                    // Decrement HL
                    ptr = ptr - 1;
                    reg[H] = ptr >> 8;
                    reg[L] = ptr;

                    setFlag(Zero, reg[B] == 0);
                    setFlag(AddSubtract, 1);

                    programCounter += 2;

                    std::clog << "IND\n";
                    return 16;
                }


                /* INDR

                    [ 1 | 0 | 1 | 1 | 1 | 0 | 1 | 0 ]
                */
                case 0b10111010:
                {
                    uint16_t ptr = pairBytes(reg[H], reg[L]);

                    uint8_t data = port_read(reg[C]);
                    mapper_write(ptr, data);

                    // Decrement B
                    reg[B] = reg[B] - 1;

                    // Increment HL
                    ptr = ptr - 1;
                    reg[H] = ptr >> 8;
                    reg[L] = ptr;

                    setFlag(Zero, 1);
                    setFlag(AddSubtract, 1);

                    std::clog << "INDR\n";

                    if(reg[B] == 0) {
                        programCounter += 2;
                        return 16;

                    }else {
                        return 21;
                    }
                }


                /* OUT (C), r
                
                    [ 0 | 1 | <   r   > | 0 | 0 | 1 ]
                */
                case 0b01111001: case 0b01000001: case 0b01001001: case 0b01010001: case 0b01011001: case 0b01100001: case 0b01101001:
                {
                    int rrr = (byte[1] & 0b00111000) >> 3;

                    uint8_t data = read_rrrSymbol(rrr);
                    port_write(reg[C], data);

                    programCounter += 2;

                    std::clog << "OUT (C), " << name_rrrSymbol(rrr) << "\n";
                    return 12;
                }


                /* OUTI

                    [ 1 | 0 | 1 | 0 | 0 | 0 | 1 | 1 ]
                */
                case 0b10100011:
                {
                    uint16_t localPtr = pairBytes(reg[H], reg[L]);

                    uint8_t data = mapper_read(localPtr);
                    port_write(reg[C], data);

                    // Decrement B
                    reg[B] = reg[B] - 1;

                    // Decrement HL
                    localPtr = localPtr + 1;
                    reg[H] = localPtr >> 8;
                    reg[L] = localPtr;

                    setFlag(Zero, reg[B] == 0);
                    setFlag(AddSubtract, 1);

                    programCounter += 2;

                    std::clog << "OUTI\n";
                    return 16;
                }


                /* OTIR

                    [ 1 | 0 | 1 | 1 | 0 | 0 | 1 | 1 ]
                */
                case 0b10110011:
                {
                    uint16_t localPtr = pairBytes(reg[H], reg[L]);

                    uint8_t data = mapper_read(localPtr);
                    port_write(reg[C], data);

                    // Decrement B
                    reg[B] = reg[B] - 1;

                    // Decrement HL
                    localPtr = localPtr + 1;
                    reg[H] = localPtr >> 8;
                    reg[L] = localPtr;

                    setFlag(Zero, 1);
                    setFlag(AddSubtract, 1);

                    std::clog << "OTIR\n";

                    if(reg[B] == 0) {
                        programCounter += 2;
                        return 16;

                    }else {
                        return 21;
                    }
                }


                /* OUTD

                    [ 1 | 0 | 1 | 0 | 1 | 0 | 1 | 1 ]
                */
                case 0b10101011:
                {
                    uint16_t localPtr = pairBytes(reg[H], reg[L]);

                    uint8_t data = mapper_read(localPtr);
                    port_write(reg[C], data);

                    // Decrement B
                    reg[B] = reg[B] - 1;

                    // Decrement HL
                    localPtr = localPtr - 1;
                    reg[H] = localPtr >> 8;
                    reg[L] = localPtr;

                    setFlag(Zero, reg[B] == 0);
                    setFlag(AddSubtract, 1);

                    programCounter += 2;

                    std::clog << "OUTD\n";
                    return 16;
                }


                /* OTDR

                    [ 1 | 0 | 1 | 1 | 1 | 0 | 1 | 1 ]
                */
                case 0b10111011:
                {
                    uint16_t localPtr = pairBytes(reg[H], reg[L]);

                    uint8_t data = mapper_read(localPtr);
                    port_write(reg[C], data);

                    // Decrement B
                    reg[B] = reg[B] - 1;

                    // Decrement HL
                    localPtr = localPtr - 1;
                    reg[H] = localPtr >> 8;
                    reg[L] = localPtr;

                    setFlag(Zero, 1);
                    setFlag(AddSubtract, 1);

                    std::clog << "OTDR\n";

                    if(reg[B] == 0) {
                        programCounter += 2;
                        return 16;

                    }else { 
                        return 21;
                    }
                }
            }
            break;
        }
    }
    return 0;
}