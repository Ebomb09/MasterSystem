#ifndef SEGA_VDP_EMULATOR_H
#define SEGA_VDP_EMULATOR_H

#include "utilities.h"
#include <functional>

struct vdp {

    vdp();

    uint8 reg[16];
    uint8 mode;

    uint8 vram[16 * 1024];
    uint8 cram[64];

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

    /* Control port */
    uint8 controlOffset;
    uint16 controlWord;

    uint8 readControlPort();
    void writeControlPort(uint8 data);
    uint8 getControlCode();
    uint16 getControlVRAMAddress();
    void incrementControlVRAMAddress();
    uint8 getControlRegisterIndex();
    uint8 getControlRegisterData();

    /* Data port */
    uint8 readBuffer;

    uint8 readDataPort();
    void writeDataPort(uint8 data);

    /* TV H/V scanners, active / inactive areas */
    uint16 vCounter;
    uint16 hCounter;

    uint8 readHCounter();
    uint8 readVCounter();
    uint16 getActiveDisplayWidth();
    uint16 getActiveDisplayHeight();
    uint16 getHCounterLimit();
    uint16 getVCounterLimit();

    /* TV screen sizes */
    uint16 getScreenWidth();
    uint16 getScreenHeight();
    uint16 getScreenOffsetX();
    uint16 getScreenOffsetY();

    /* VRAM addresses */
    uint16 getNameTableBaseAddress();
    uint16 getSpriteTableBaseAddress();

    int frameBuffer[256][240];

    void drawScanLine();
    void drawTile(uint16 tileIndex, int x, int y, bool horizontalFlip=false, bool verticalFlip=false, bool spritePalette = false, bool doubleScale = false, bool tileWrap = false);
    void drawTilemap(bool drawPriority);
    void drawSprites();

    int getColor(uint8 paletteIndex);

    /* Inputs from console */
    std::function<int()> getDeviceType;
};

#endif