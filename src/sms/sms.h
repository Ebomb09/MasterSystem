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
    uint8 ram[8 * 1024];
    uint8 sram[2][16 * 1024];

    uint8 joypad1;
    uint8 joypad2;

    uint8 joypadControl;
    uint8 memoryControl;

    bool loadRom(std::string romPath);

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

    void draw(SDL_Renderer* renderer);
};

#endif