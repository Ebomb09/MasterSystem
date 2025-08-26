#ifndef SEGA_VDP_EMULATOR_H
#define SEGA_VDP_EMULATOR_H

#include "utilities.h"
#include <functional>

struct vdp {

    vdp();

    uint8 reg[16];
    uint8 mode;

    uint8 vram[16 * 1024];
    uint8 cram[32];

    uint8 controlOffset;
    uint16 control;
    uint8 buffer;

    uint16 vcounter;
    uint16 hcounter;
    enum StatusFlags {
        VBlank              = 0b10000000,
        SpriteOverflow      = 0b01000000,
        SpriteCollision     = 0b00100000
    };
    uint8 status;

    bool cycle();
    bool canSendInterrupt();

    bool requestLineInterrupt;
    bool requestFrameInterrupt;

    uint8 readControlPort();
    void writeControlPort(uint8 data);

    uint8 readDataPort();
    void writeDataPort(uint8 data);

    uint8 getControlCode();
    uint16 getControlVRAMAddress();
    void incrementControlVRAMAddress();
    uint8 getControlRegisterIndex();
    uint8 getControlRegisterData();

    void scanSprites();
};

#endif