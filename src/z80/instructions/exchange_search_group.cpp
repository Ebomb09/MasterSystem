#include "../z80.h"
#include <algorithm>
#include <iostream>

int z80::processExchangeSearchGroup() {

    uint8 byte[4] {
        mapper_read(programCounter),
        mapper_read(programCounter+1),
        mapper_read(programCounter+2),
        mapper_read(programCounter+3)
    };

    switch(byte[0]) {

        /* EX DE, HL
        
            [ 1 | 1 | 1 | 0 | 1 | 0 | 1 | 1 ]
        */
        case 0b11101011:
        {
            std::swap(reg[D], reg[H]);
            std::swap(reg[E], reg[L]);
            programCounter += 1;

            std::clog << "EX DE, HL\n";
            return 4;
        }


        /* EX AF, AF'
        
            [ 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 ]
        */
        case 0b00001000:
        {
            std::swap(reg[A], reg[A_p]);
            std::swap(reg[F], reg[F_p]);
            programCounter += 1;

            std::clog << "EX AF, AF'\n";
            return 4;
        }


        /* EXX
        
            [ 1 | 1 | 0 | 1 | 1 | 0 | 0 | 1 ]
        */
        case 0b11011001:
        {
            std::swap(reg[B], reg[B_p]);
            std::swap(reg[C], reg[C_p]);
            std::swap(reg[D], reg[D_p]);
            std::swap(reg[E], reg[E_p]);
            std::swap(reg[H], reg[H_p]);
            std::swap(reg[L], reg[L_p]);
            programCounter += 1;

            std::clog << "EXX\n";
            return 4;
        }


        /* EX (SP), HL
        
            [ 1 | 1 | 1 | 0 | 0 | 0 | 1 | 1 ]
        */
        case 0b11100011:
        {
            uint8 hi = reg[H];
            uint8 lo = reg[L];

            reg[L] = mapper_read(stackPointer);
            reg[H] = mapper_read(stackPointer+1);

            mapper_write(stackPointer, lo);
            mapper_write(stackPointer+1, hi);

            programCounter += 1;

            std::clog << "EX (SP), HL\n";
            return 19;
        }


        /* IX Opcodes

            [ 1 | 1 | 0 | 1 | 1 | 1 | 0 | 1 ]
        */
        case 0b11011101:
        {

            switch(byte[1]) {

                /* EX (SP), IX
                
                    [ 1 | 1 | 1 | 0 | 0 | 0 | 1 | 1 ]
                */
                case 0b11100011:
                {
                    uint8 hi = indexRegisterX >> 8;
                    uint8 lo = indexRegisterX;
        
                    indexRegisterX = pairBytes(mapper_read(stackPointer+1), mapper_read(stackPointer));
        
                    mapper_write(stackPointer, lo);
                    mapper_write(stackPointer+1, hi);

                    programCounter += 2;

                    std::clog << "EX (SP), IX\n";
                    return 23;
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

                /* EX (SP), IY
                
                    [ 1 | 1 | 1 | 0 | 0 | 0 | 1 | 1 ]
                */
                case 0b11100011:
                {
                    uint8 hi = indexRegisterY >> 8;
                    uint8 lo = indexRegisterY;
        
                    indexRegisterY = pairBytes(mapper_read(stackPointer+1), mapper_read(stackPointer));
        
                    mapper_write(stackPointer, lo);
                    mapper_write(stackPointer+1, hi);

                    programCounter += 2;

                    std::clog << "EX (SP), IY\n";
                    return 23;
                }
            }
            break;
        }


        /* Load Opcodes
        
            [ 1 | 1 | 1 | 0 | 1 | 1 | 0 | 1 ]
        */
        case 0b11101101: 
        {

            switch(byte[1]) {

                /* LDI
                
                    [ 1 | 0 | 1 | 0 | 0 | 0 | 0 | 0 ]
                */
                case 0b10100000:
                {
                    mapper_write(pairBytes(reg[D], reg[E]), mapper_read(pairBytes(reg[H], reg[L])));

                    uint16 inc = pairBytes(reg[D], reg[E]) + 1;
                    reg[D] = inc >> 8;
                    reg[E] = inc;

                    inc = pairBytes(reg[H], reg[L]) + 1;
                    reg[H] = inc >> 8;
                    reg[L] = inc;

                    inc = pairBytes(reg[B], reg[C]) - 1;
                    reg[B] = inc >> 8;
                    reg[C] = inc;

                    setFlag(HalfCarry, 0);
                    setFlag(ParityOverflow, pairBytes(reg[B], reg[C]) != 0);
                    setFlag(AddSubtract, 0);

                    programCounter += 2;
                    std::clog << "LDI\n";
                    return 16;
                }


                /* LDIR
                
                    [ 1 | 0 | 1 | 1 | 0 | 0 | 0 | 0 ]
                */
                case 0b10110000:
                {
                    mapper_write(pairBytes(reg[D], reg[E]), mapper_read(pairBytes(reg[H], reg[L])));

                    uint16 inc = pairBytes(reg[D], reg[E]) + 1;
                    reg[D] = inc >> 8;
                    reg[E] = inc;

                    inc = pairBytes(reg[H], reg[L]) + 1;
                    reg[H] = inc >> 8;
                    reg[L] = inc;

                    inc = pairBytes(reg[B], reg[C]) - 1;
                    reg[B] = inc >> 8;
                    reg[C] = inc;

                    setFlag(HalfCarry, 0);
                    setFlag(ParityOverflow, pairBytes(reg[B], reg[C]) != 0);
                    setFlag(AddSubtract, 0);

                    std::clog << "LDIR\n";

                    if(pairBytes(reg[B], reg[C]) == 0) {
                        programCounter += 2;
                        return 16;

                    }else {
                        return 21;
                    }
                }


                /* LDD
                
                    [ 1 | 0 | 1 | 0 | 1 | 0 | 0 | 0 ]
                */
                case 0b10101000:
                {
                    mapper_write(pairBytes(reg[D], reg[E]), mapper_read(pairBytes(reg[H], reg[L])));

                    uint16 inc = pairBytes(reg[D], reg[E]) - 1;
                    reg[D] = inc >> 8;
                    reg[E] = inc;

                    inc = pairBytes(reg[H], reg[L]) - 1;
                    reg[H] = inc >> 8;
                    reg[L] = inc;

                    inc = pairBytes(reg[B], reg[C]) - 1;
                    reg[B] = inc >> 8;
                    reg[C] = inc;

                    setFlag(HalfCarry, 0);
                    setFlag(ParityOverflow, pairBytes(reg[B], reg[C]) != 0);
                    setFlag(AddSubtract, 0);

                    programCounter += 2;

                    std::clog << "LDD\n";
                    return 16;
                }


                /* LDDR
                
                    [ 1 | 0 | 1 | 1 | 1 | 0 | 0 | 0 ]
                */
                case 0b10111000:
                {
                    mapper_write(pairBytes(reg[D], reg[E]), mapper_read(pairBytes(reg[H], reg[L])));

                    uint16 inc = pairBytes(reg[D], reg[E]) - 1;
                    reg[D] = inc >> 8;
                    reg[E] = inc;

                    inc = pairBytes(reg[H], reg[L]) - 1;
                    reg[H] = inc >> 8;
                    reg[L] = inc;

                    inc = pairBytes(reg[B], reg[C]) - 1;
                    reg[B] = inc >> 8;
                    reg[C] = inc;

                    setFlag(HalfCarry, 0);
                    setFlag(ParityOverflow, pairBytes(reg[B], reg[C]) != 0);
                    setFlag(AddSubtract, 0);

                    std::clog << "LDDR\n";

                    if(pairBytes(reg[B], reg[C]) == 0) {
                        programCounter += 2;
                        return 16;

                    }else {
                        return 21;
                    }
                }


                /* CPI
                
                    [ 1 | 0 | 1 | 0 | 0 | 0 | 0 | 1 ]
                */
                case 0b10100001:
                {
                    uint8 a = reg[A];
                    uint8 b = mapper_read(pairBytes(reg[H], reg[L]));
                    uint8 comp = a - b;
                    
                    uint16 inc = pairBytes(reg[H], reg[L]) + 1;
                    reg[H] = inc >> 8;
                    reg[L] = inc;

                    inc = pairBytes(reg[B], reg[C]) - 1;
                    reg[B] = inc >> 8;
                    reg[C] = inc;
                    
                    setFlag(Sign, comp & 0b10000000);
                    setFlag(Zero, comp == 0);
                    setFlag(HalfCarry, halfBorrow8(a, b));
                    setFlag(ParityOverflow, pairBytes(reg[B], reg[C]) != 0);
                    setFlag(AddSubtract, 1);

                    programCounter += 2;

                    std::clog << "CPI\n";
                    return 16;
                }


                /* CPIR
                
                    [ 1 | 0 | 1 | 1 | 0 | 0 | 0 | 1 ]
                */
                case 0b10110001:
                {
                    uint8 a = reg[A];
                    uint8 b = mapper_read(pairBytes(reg[H], reg[L]));
                    uint8 comp = a - b;
                    
                    uint16 inc = pairBytes(reg[H], reg[L]) + 1;
                    reg[H] = inc >> 8;
                    reg[L] = inc;

                    inc = pairBytes(reg[B], reg[C]) - 1;
                    reg[B] = inc >> 8;
                    reg[C] = inc;

                    setFlag(Sign, comp & 0b10000000);
                    setFlag(Zero, comp == 0);
                    setFlag(HalfCarry, halfBorrow8(a, b));
                    setFlag(ParityOverflow, pairBytes(reg[B], reg[C]) != 0);
                    setFlag(AddSubtract, 1);

                    std::clog << "CPIR\n";

                    if(pairBytes(reg[B], reg[C]) == 0 || comp == 0) {
                        programCounter += 2;
                        return 16;

                    }else {
                        return 21;
                    }
                }


                /* CPD
                
                    [ 1 | 0 | 1 | 0 | 1 | 0 | 0 | 1 ]
                */
                case 0b10101001:
                {
                    uint8 a = reg[A];
                    uint8 b = mapper_read(pairBytes(reg[H], reg[L]));
                    uint8 comp = a - b;
                    
                    uint16 inc = pairBytes(reg[H], reg[L]) - 1;
                    reg[H] = inc >> 8;
                    reg[L] = inc;

                    inc = pairBytes(reg[B], reg[C]) - 1;
                    reg[B] = inc >> 8;
                    reg[C] = inc;
                    
                    setFlag(Sign, comp & 0b10000000);
                    setFlag(Zero, comp == 0);
                    setFlag(HalfCarry, halfBorrow8(a, b));
                    setFlag(ParityOverflow, pairBytes(reg[B], reg[C]) != 0);
                    setFlag(AddSubtract, 1);

                    programCounter += 2;

                    std::clog << "CPD\n";
                    return 16;
                }


                /* CPDR
                
                    [ 1 | 0 | 1 | 1 | 1 | 0 | 0 | 1 ]
                */
                case 0b10111001:
                {
                    uint8 a = reg[A];
                    uint8 b = mapper_read(pairBytes(reg[H], reg[L]));
                    uint8 comp = a - b;
                    
                    uint16 inc = pairBytes(reg[H], reg[L]) - 1;
                    reg[H] = inc >> 8;
                    reg[L] = inc;

                    inc = pairBytes(reg[B], reg[C]) - 1;
                    reg[B] = inc >> 8;
                    reg[C] = inc;

                    setFlag(Sign, comp & 0b10000000);
                    setFlag(Zero, comp == 0);
                    setFlag(HalfCarry, halfBorrow8(a, b));
                    setFlag(ParityOverflow, pairBytes(reg[B], reg[C]) != 0);
                    setFlag(AddSubtract, 1);

                    std::clog << "CPDR\n";

                    if(pairBytes(reg[B], reg[C]) == 0 || comp == 0) {
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