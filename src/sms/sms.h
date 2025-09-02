#ifndef SMS_STRUCT_H
#define SMS_STRUCT_H

#include "z80/z80.h"
#include "vdp/vdp.h"
#include "psg/psg.h"

#include <iostream>
#include <string>

#include "SDL3/SDL.h"

struct sms {

    sms();
    ~sms();

    z80 cpu;
    vdp gpu;
    psg audio;

    uint8* rom = NULL;
    int romSize;
    int romHeader;
    uint8 ram[8 * 1024];
    uint8 sram[2][16 * 1024];

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
    uint8 joypad1;
    uint8 joypad2;
    uint8 joypadStart;
    void setJoyPadControl(uint8 control, bool val);

    bool loadRom(std::string romPath);


    enum MapperFlags {
        ROM_BankShift           = 0b00000011,
        SRAM_BankSelect         = 0b00000100,
        SRAM_EnableSlot2        = 0b00001000,
        SRAM_EnableRAM          = 0b00010000,
        ROM_EnableWrite         = 0b10000000
    };
    uint8 mapperOptions;
    uint8 mapperBankSelect[3];
    uint8 mapper_read(uint16 addr);
    void mapper_write(uint16 addr, uint8 data);

    enum Ports {
        ControllerAPort     = 0xDC,
        ControllerBPort     = 0xDD,
        ControlPort         = 0xBF,
        DataPort            = 0xBE,
        VCounterPort        = 0x7E,
        HCounterPort        = 0x7F
    };
    uint8 port_read(uint16 addr);
    void port_write(uint16 addr, uint8 data);

    int update(SDL_Renderer* renderer, SDL_AudioStream* stream);

    SDL_Texture* frame;
    void draw(SDL_Renderer* renderer);

    int deviceType;
    int getMasterClock();
};

#endif