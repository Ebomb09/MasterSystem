#ifndef Z80_CPU_H
#define Z80_CPU_H

#include <cstdint>
#include <functional>

struct Z80 {

    Z80();

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

    uint8_t reg[16];

    uint8_t interruptVector;
    uint8_t memoryRefresh;

    uint16_t indexRegisterX;
    uint16_t indexRegisterY;

    uint16_t stackPointer;
    uint16_t programCounter;

    uint8_t interruptMode;
    uint8_t IFF1, IFF2;

    enum HaltState {
        HALT_NONE, HALT_WAIT, HALT_GOOD
    };
    uint8_t haltState;

    enum EIState {
        EI_NONE, EI_WAIT, EI_GOOD
    };
    uint8_t eiState;

    std::function<uint8_t(uint16_t)>        port_read;
    std::function<void(uint16_t, uint8_t)> port_write;

    std::function<uint8_t(uint16_t)>        mapper_read;
    std::function<void(uint16_t, uint8_t)> mapper_write;

public:
    void setFlag(uint8_t flag, bool val);
    bool getFlag(uint8_t flag);

    int cycle();

    void signalNMI();
    void signalINT();

private:
    int process8BitLoadGroup();

    // 16bit Load Group
    int process16BitLoadGroup();
    void PUSH(const uint16_t& data);
    void POP(uint16_t& data);

    int processExchangeSearchGroup();
    int processGeneralArithmeticGroup();
    int processJumpGroup();
    int processInputOutputGroup();

    // 8bit Arithmetic
    int process8BitArithmeticGroup();
    void ADD(uint8_t& a, const uint8_t& b);
    void ADC(uint8_t& a, const uint8_t& b);
    void SUB(uint8_t& a, const uint8_t& b);
    void SBC(uint8_t& a, const uint8_t& b);
    void AND(uint8_t& a, const uint8_t& b);
    void OR(uint8_t& a, const uint8_t& b);
    void XOR(uint8_t& a, const uint8_t& b);
    void CP(const uint8_t& a, const uint8_t& b);
    void INC(uint8_t& a);
    void DEC(uint8_t& a);

    //16bit Arithmetic
    int process16BitArithmeticGroup();
    void ADD16(uint16_t& a, const uint16_t& b);
    void ADC16(uint16_t& a, const uint16_t& b);
    void SBC16(uint16_t& a, const uint16_t& b);
    void INC16(uint16_t& a);
    void DEC16(uint16_t& a);

    // Call, return
    int processCallReturnGroup();
    void CALL(const uint16_t& addr);
    void RST(const uint8_t& p);
    void RET();
    void RETI();
    void RETN();

    // Bit Set, Reset, Test
    int processBitSetResetTest();
    void BIT(const uint8_t& bit, const uint8_t& data);
    void SET(const uint8_t& bit, uint8_t& data);
    void RES(const uint8_t& bit, uint8_t& data);

    // Rotate, Shift
    int processRotateShiftGroup();
    void RLC(uint8_t& num, bool A = false);
    void RL(uint8_t& num, bool A = false);
    void RRC(uint8_t& num, bool A = false);
    void RR(uint8_t& num, bool A = false);
    void SLA(uint8_t& num);
    void SRA(uint8_t& num);
    void SRL(uint8_t& num);
    void SLL(uint8_t& num);

    // Helper
    void incrementPC(int val);

    uint8_t read_rrrSymbol(uint8_t rrr);
    void write_rrrSymbol(uint8_t rrr, uint8_t data);
    const char* name_rrrSymbol(uint8_t rrr);

    uint16_t read_ddSymbol(uint8_t dd);
    void write_ddSymbol(uint8_t dd, uint16_t data);
    const char* name_ddSymbol(uint8_t dd);

    uint16_t read_ssSymbol(uint8_t ss);
    void write_ssSymbol(uint8_t ss, uint16_t data);
    const char* name_ssSymbol(uint8_t ss);

    uint16_t read_qqSymbol(uint8_t qq);
    void write_qqSymbol(uint8_t qq, uint16_t data);
    const char* name_qqSymbol(uint8_t qq);

    uint16_t read_ppSymbol(uint8_t pp);
    void write_ppSymbol(uint8_t pp, uint16_t data);
    const char* name_ppSymbol(uint8_t pp);

    uint16_t read_rrSymbol(uint8_t rr);
    void write_rrSymbol(uint8_t rr, uint16_t data);
    const char* name_rrSymbol(uint8_t rr);

    bool read_cccSymbol(uint8_t ccc);
    const char* name_cccSymbol(uint8_t ccc);
};

#endif