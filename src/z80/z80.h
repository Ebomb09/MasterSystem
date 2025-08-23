#ifndef Z80_EMULATOR_H
#define Z80_EMULATOR_H

#include "utilities.h"
#include <functional>
#include <string>

struct z80 {

    z80();

    enum RegisterNames {
        A, F, B, C, D, E, H, L, A_p, F_p, B_p, C_p, D_p, E_p, H_p, L_p
    };

    enum StatusFlag {
        Sign            = 0b10000000,
        Zero            = 0b01000000,
        HalfCarry       = 0b00010000,
        ParityOverflow  = 0b00000100,
        AddSubtract     = 0b00000010,
        Carry           = 0b00000001
    };

    uint8 reg[16];

    uint8 interruptVector;
    uint8 memoryRefresh;

    uint16 indexRegisterX;
    uint16 indexRegisterY;

    uint16 stackPointer;
    uint16 programCounter;

    uint8 interruptMode;
    uint8 IFF1, IFF2;

    uint8 signal_NMI = 1;
    uint8 signal_INT = 1;

    std::function<uint8(uint16)>        port_read;
    std::function<void(uint16, uint8)> port_write;

    std::function<uint8(uint16)>        mapper_read;
    std::function<void(uint16, uint8)> mapper_write;

public:
    void setFlag(uint8 flag, bool val);
    bool getFlag(uint8 flag);

    int cycle();

private:
    int processInterupt();

    int process8BitLoadGroup();

    // 16bit Load Group
    int process16BitLoadGroup();
    void PUSH(const uint16& data);
    void POP(uint16& data);

    int processExchangeSearchGroup();
    int processGeneralArithmeticGroup();
    int processJumpGroup();
    int processInputOutputGroup();

    // 8bit Arithmetic
    int process8BitArithmeticGroup();
    void ADD(uint8& a, const uint8& b);
    void ADC(uint8& a, const uint8& b);
    void SUB(uint8& a, const uint8& b);
    void SBC(uint8& a, const uint8& b);
    void AND(uint8& a, const uint8& b);
    void OR(uint8& a, const uint8& b);
    void XOR(uint8& a, const uint8& b);
    void CP(const uint8& a, const uint8& b);
    void INC(uint8& a);
    void DEC(uint8& a);

    //16bit Arithmetic
    int process16BitArithmeticGroup();
    void ADD16(uint16& a, const uint16& b);
    void ADC16(uint16& a, const uint16& b);
    void SBC16(uint16& a, const uint16& b);
    void INC16(uint16& a);
    void DEC16(uint16& a);

    // Call, return
    int processCallReturnGroup();
    void CALL(const uint16& addr);
    void RST(const uint8& p);
    void RET();
    void RETI();
    void RETN();

    // Bit Set, Reset, Test
    int processBitSetResetTest();
    void BIT(const uint8& bit, const uint8& data);
    void SET(const uint8& bit, uint8& data);
    void RES(const uint8& bit, uint8& data);

    // Rotate, Shift
    int processRotateShiftGroup();
    void RLC(uint8& num, bool A = false);
    void RL(uint8& num, bool A = false);
    void RRC(uint8& num, bool A = false);
    void RR(uint8& num, bool A = false);
    void SLA(uint8& num);
    void SRA(uint8& num);
    void SRL(uint8& num);
    void SLL(uint8& num);

    // Helper
    void incrementPC(int val);

    uint8 read_rrrSymbol(uint8 rrr);
    void write_rrrSymbol(uint8 rrr, uint8 data);
    std::string name_rrrSymbol(uint8 rrr);

    uint16 read_ddSymbol(uint8 dd);
    void write_ddSymbol(uint8 dd, uint16 data);
    std::string name_ddSymbol(uint8 dd);

    uint16 read_ssSymbol(uint8 ss);
    void write_ssSymbol(uint8 ss, uint16 data);
    std::string name_ssSymbol(uint8 ss);

    uint16 read_qqSymbol(uint8 qq);
    void write_qqSymbol(uint8 qq, uint16 data);
    std::string name_qqSymbol(uint8 qq);

    uint16 read_ppSymbol(uint8 pp);
    void write_ppSymbol(uint8 pp, uint16 data);
    std::string name_ppSymbol(uint8 pp);

    uint16 read_rrSymbol(uint8 rr);
    void write_rrSymbol(uint8 rr, uint16 data);
    std::string name_rrSymbol(uint8 rr);

    bool read_cccSymbol(uint8 ccc);
    std::string name_cccSymbol(uint8 ccc);
};

#endif