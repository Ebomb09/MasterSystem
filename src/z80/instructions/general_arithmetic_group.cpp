#include "../z80.h"
#include <bitset>
#include <iostream>

int z80::processGeneralArithmeticGroup() {

    uint8 byte[4] {
        mapper_read(programCounter),
        mapper_read(programCounter+1),
        mapper_read(programCounter+2),
        mapper_read(programCounter+3)
    };

    switch(byte[0]) {

        /* DAA

            [ 0 | 0 | 1 | 0 | 0 | 1 | 1 | 1 ]
        */  
        case 0b00100111:
        {
            incrementPC(1);

            // Check DAA instruction table for values for Binary Coded Decimals
            uint8 fix = 0;

            if(getFlag(HalfCarry) || (reg[A] & 0x0F) >= 0x0A) {
                fix += 0x06;

                // Adjust the new HalfCarry flag
                if(getFlag(AddSubtract)) {

                    if(getFlag(HalfCarry)) {
                        setFlag(HalfCarry, (reg[A] & 0x0f) < 0x06);
                    }
                }else{
                    setFlag(HalfCarry, (reg[A] & 0x0f) >= 0x0A);
                }
            }
             
            if(getFlag(Carry) || reg[A] >= 0x9A) {
                fix += 0x60;
                setFlag(Carry, 1);
            }
            
            if(getFlag(AddSubtract)) {
                fix = fix * -1;
            }

            reg[A] = reg[A] + fix;

            setFlag(Sign, reg[A] & 0b10000000);
            setFlag(Zero, reg[A] == 0);
            setFlag(ParityOverflow, std::bitset<8>(reg[A]).count() % 2 == 0);

            std::clog << "DAA\n";
            return 4;
        }


        /* CPL

            [ 0 | 0 | 1 | 0 | 1 | 1 | 1 | 1 ]
        */  
        case 0b00101111:
        {
            incrementPC(1);
            reg[A] = ~reg[A];
            setFlag(HalfCarry, 1);
            setFlag(AddSubtract, 1);
            std::clog << "CPL\n";
            return 4;
        }


        /* CCF
        
            [ 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 ]
        */
        case 0b00111111:
        {
            incrementPC(1);

            if(getFlag(Carry)) {
                setFlag(HalfCarry, 1);
                setFlag(Carry, 0);

            }else{
                setFlag(HalfCarry, 0);
                setFlag(Carry, 1);
            }
            setFlag(AddSubtract, 0);

            std::clog << "CCF\n";
            return 4;
        }


        /* SCF
        
            [ 0 | 0 | 1 | 1 | 0 | 1 | 1 | 1 ]
        */
        case 0b00110111:
        {
            incrementPC(1);
            setFlag(HalfCarry, 0);
            setFlag(AddSubtract, 0);
            setFlag(Carry, 1);
            std::clog << "SCF\n";
            return 4;
        }


        /* NOP
        
            [ 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 ]
        */
        case 0b00000000:
        {
            incrementPC(1);
            std::clog << "NOP\n";
            return 4;
        }


        /* HALT
        
            [ 0 | 1 | 1 | 1 | 0 | 1 | 1 | 0 ]
        */
        case 0b01110110:
        {

            if(signal_NMI == 0 || signal_INT) 
                incrementPC(1);

            std::clog << "HALT\n";
            return 4;
        }


        /* DI
        
            [ 1 | 1 | 1 | 1 | 0 | 0 | 1 | 1 ]
        */
        case 0b11110011:
        {
            incrementPC(1);
            IFF1 = 0;
            IFF2 = 0;
            std::clog << "DI\n";
            return 4;
        }


        /* EI
        
            [ 1 | 1 | 1 | 1 | 1 | 0 | 1 | 1 ]
        */
        case 0b11111011:
        {
            incrementPC(1);
            IFF1 = 1;
            IFF2 = 1;
            std::clog << "EI\n";
            return 4;
        }


        /* Load Opcodes

            [ 1 | 1 | 1 | 0 | 1 | 1 | 0 | 1 ]
        */  
        case 0b11101101:
        {

            switch(byte[1]) {

                /* NEG

                    [ 0 | 1 | 0 | 0 | 0 | 1 | 0 | 0 ]
                */
                case 0b01000100:
                {
                    incrementPC(2);

                    // TODO
                    uint8 a = 0;
                    uint8 b = reg[A];

                    reg[A] = a - b;

                    setFlag(Sign, reg[A] & 0b10000000);
                    setFlag(Zero, reg[A] == 0);
                    setFlag(ParityOverflow, b == 0x80);
                    setFlag(AddSubtract, 1);
                    setFlag(HalfCarry, halfCarry8(a, -b));
                    setFlag(Carry, b == 0x0);

                    std::clog << "NEG\n";
                    return 8;
                }

                /* IM 0
                
                    [ 0 | 1 | 0 | 0 | 0 | 1 | 1 | 0 ]
                */
                case 0b01000110:
                {
                    incrementPC(2);
                    interruptMode = 0;
                    std::clog << "IM 0\n";
                    return 8;
                }


                /* IM 1
                
                    [ 0 | 1 | 0 | 1 | 0 | 1 | 1 | 0 ]
                */
                case 0b01010110:
                {
                    incrementPC(2);
                    interruptMode = 1;
                    std::clog << "IM 1\n";
                    return 8;
                }


                /* IM 2
                
                    [ 0 | 1 | 0 | 1 | 1 | 1 | 1 | 0 ]
                */
                case 0b01011110:
                {
                    incrementPC(2);
                    interruptMode = 2;
                    std::clog << "IM 2\n";
                    return 8;
                }
            }
            break;
        }
    }
    return 0;
}