#ifndef TMS9918A_PROCESSOR_H
#define TMS9918A_PROCESSOR_H

#include <cstdint>
#include <functional>

struct TMS9918A {

    TMS9918A();

    uint8_t reg[16];
    uint8_t mode;

    uint8_t vram[16 * 1024];
    uint8_t cram[64];

    enum StatusFlags {
        VBlank              = 0b10000000,
        SpriteOverflow      = 0b01000000,
        SpriteCollision     = 0b00100000
    };
    uint8_t status;

    enum VideoFormat {
        MASTERSYSTEM_NTSC, MASTERSYSTEM_PAL, GAMEGEAR_NTSC
    };
    uint8_t videoFormat;

    bool cycle();
    bool canSendInterrupt();

    bool requestLineInterrupt;
    bool requestFrameInterrupt;

    /* Control port */
    uint8_t controlOffset;
    uint16_t controlWord;

    uint8_t readControlPort();
    void writeControlPort(uint8_t data);
    uint8_t getControlCode();
    uint16_t getControlVRAMAddress();
    void incrementControlVRAMAddress();
    uint8_t getControlRegisterIndex();
    uint8_t getControlRegisterData();

    /* Data port */
    uint8_t readBuffer;

    uint8_t readDataPort();
    void writeDataPort(uint8_t data);

    /* TV H/V scanners, active / inactive areas */
    uint16_t vCounter;
    uint16_t hCounter;

    uint8_t readHCounter();
    uint8_t readVCounter();
    uint16_t getActiveDisplayWidth();
    uint16_t getActiveDisplayHeight();
    uint16_t getHCounterLimit();
    uint16_t getVCounterLimit();

    /* TV screen sizes */
    uint16_t getScreenWidth();
    uint16_t getScreenHeight();
    uint16_t getScreenOffsetX();
    uint16_t getScreenOffsetY();

    /* VRAM addresses */
    uint16_t getNameTableBaseAddress();
    uint16_t getSpriteTableBaseAddress();

    int frameBuffer[256 * 240];

    void drawScanLine();
    void drawTile(uint16_t tileIndex, int x, int y, bool horizontalFlip=false, bool verticalFlip=false, bool spritePalette = false, bool doubleScale = false, bool tileWrap = false);
    void drawTilemap(bool drawPriority);
    void drawSprites();
    int getColor(uint8_t paletteIndex);
};

#endif