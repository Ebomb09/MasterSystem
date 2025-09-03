#ifndef SMS_STRUCT_H
#define SMS_STRUCT_H

#include "Z80/Z80.h"
#include "TMS9918A/TMS9918A.h"
#include "SN76489/SN76489.h"

#include <string>

#include "SDL3/SDL.h"

struct sms {

    sms();
    ~sms();

    Z80 cpu;
    TMS9918A gpu;
    SN76489 psg;

    uint8_t* rom = NULL;
    int romSize;
    int romHeader;
    uint8_t ram[8 * 1024];
    uint8_t sram[2][16 * 1024];

    enum {
        Joypad_B_Down,
        Joypad_B_Up,
        Joypad_A_TR,
        Joypad_A_TL,
        Joypad_A_Right,
        Joypad_A_Left,
        Joypad_A_Down,
        Joypad_A_Up,
        Joypad_B_TH,
        Joypad_A_TH,
        Console_Reset,
        Joypad_B_TR,
        Joypad_B_TL,
        Joypad_B_Right,
        Joypad_B_Left
    };
    uint8_t joypad1;
    uint8_t joypad2;
    uint8_t joypadStart;
    void setJoyPadControl(uint8_t control, bool val);

    bool loadRom(std::string romPath);


    enum MapperFlags {
        ROM_BankShift           = 0b00000011,
        SRAM_BankSelect         = 0b00000100,
        SRAM_EnableSlot2        = 0b00001000,
        SRAM_EnableRAM          = 0b00010000,
        ROM_EnableWrite         = 0b10000000
    };
    uint8_t mapperOptions;
    uint8_t mapperBankSelect[3];
    uint8_t mapper_read(uint16_t addr);
    void mapper_write(uint16_t addr, uint8_t data);

    enum Ports {
        ControllerAPort     = 0xDC,
        ControllerBPort     = 0xDD,
        ControlPort         = 0xBF,
        DataPort            = 0xBE,
        VCounterPort        = 0x7E,
        HCounterPort        = 0x7F
    };
    uint8_t port_read(uint16_t addr);
    void port_write(uint16_t addr, uint8_t data);

    int update(SDL_Renderer* renderer, SDL_AudioStream* stream);

    SDL_Texture* frame;
    void draw(SDL_Renderer* renderer);

    int getMasterClock();
};

#endif