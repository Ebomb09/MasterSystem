#include "TMS9918A.h"
#include "utilities.h"

TMS9918A::TMS9918A() {
    controlOffset = 0;

    requestFrameInterrupt = false;
    requestLineInterrupt = false;

    for(int i = 0; i < 16; i ++)
        reg[i] = 0xFF;

    hCounter = 0;
    vCounter = 0;

    mode = 4;

    videoFormat = MASTERSYSTEM_NTSC;
}

void TMS9918A::writeControlPort(uint8_t data) {

    if(controlOffset == 0) {
        controlWord = (controlWord & 0xFF00) | data;
        controlOffset = 1;

    }else {
        controlWord = (controlWord & 0x00FF) | ((uint16_t)data << 8);
        controlOffset = 0;

        switch(getControlCode()) {

            // VRAM read
            case 0:
            {
                readBuffer = vram[getControlVRAMAddress()];
                incrementControlVRAMAddress();
                break;
            }

            // VRAM write
            case 1:
            {
                readBuffer = vram[getControlVRAMAddress()];
                break;
            }

            // Register write
            case 2:
            {
                reg[getControlRegisterIndex()] = getControlRegisterData();
                break;
            }
        }
    }
}

uint8_t TMS9918A::readControlPort() {
    controlOffset = 0;

    requestFrameInterrupt = false;
    requestLineInterrupt = false;

    // Return status and reset flags
    uint8_t res = status;
    status = 0;
    return res;
}

uint8_t TMS9918A::readDataPort() {
    controlOffset = 0;

    // Return the value in the current buffer and then update, and increment
    uint8_t res = readBuffer;

    readBuffer = vram[getControlVRAMAddress()];
    incrementControlVRAMAddress();
    
    return res;
}

void TMS9918A::writeDataPort(uint8_t data) {
    controlOffset = 0;

    switch(getControlCode()) {

        case 0:
        case 1:
        case 2:
        {
            vram[getControlVRAMAddress()] = data;
            readBuffer = data;
            incrementControlVRAMAddress();
            break;
        }

        case 3:
        {
            cram[getControlVRAMAddress() % 64] = data;
            incrementControlVRAMAddress();
            break;
        }
    }
}

uint8_t TMS9918A::getControlCode() {
    return (controlWord & 0b1100000000000000) >> 14;
}

uint16_t TMS9918A::getControlVRAMAddress() {
    return (controlWord & 0b0011111111111111);
}

void TMS9918A::incrementControlVRAMAddress() {
    uint16_t addr = getControlVRAMAddress();
    addr += 1;

    // Bound the address
    if(addr > 0x3FFF)
        addr = 0;

    controlWord = (controlWord & 0b1100000000000000) + addr;
}

uint8_t TMS9918A::getControlRegisterIndex() {
    return (controlWord & 0b0000111100000000) >> 8;
}

uint8_t TMS9918A::getControlRegisterData() {
    return (controlWord & 0b0000000011111111);
}

bool TMS9918A::cycle() {
    bool clearVBlank = false;
    bool enableLineInterrupts   = reg[0x0] & 0b00010000;
    bool enableFrameInterrupts  = reg[0x1] & 0b00100000;

    hCounter ++;

    // Cleared HBlank
    if(hCounter >= getHCounterLimit()) {
        hCounter = 0;
        vCounter ++;
    }

    // Cleared VBlank
    if(vCounter >= getVCounterLimit()) {
        clearVBlank = true;
        vCounter = 0;
    }

    // Render the scanline
    if(vCounter < getActiveDisplayHeight() && hCounter == getActiveDisplayWidth()) {
        drawScanLine();
    }

    // Cleared Active Display
    if(vCounter == getActiveDisplayHeight()-1 && hCounter == getActiveDisplayWidth()) {
        status |= VBlank;
        if(enableFrameInterrupts) requestFrameInterrupt = true;
    }

    // Cleared LineCounter
    if(reg[0xA] > 0 && vCounter > 0 && vCounter % reg[0xA] == 0 && vCounter < getActiveDisplayHeight() && hCounter == getActiveDisplayWidth()) {
        if(enableLineInterrupts) requestLineInterrupt = true;
    }
    return clearVBlank;
}

bool TMS9918A::canSendInterrupt() {
    bool enableLineInterrupts   = reg[0x0] & 0b00010000;
    bool enableFrameInterrupts  = reg[0x1] & 0b00100000;

    return (enableFrameInterrupts && requestFrameInterrupt) || (enableLineInterrupts && requestLineInterrupt);
}

uint16_t TMS9918A::getActiveDisplayWidth() {
    return 256;
}

uint16_t TMS9918A::getActiveDisplayHeight() {

    if(reg[0x1] & 0b00001000) {
        return 240;

    }else if(reg[0x1] & 0b00010000) {
        return 224;
    }
    return 192;
}

uint16_t TMS9918A::getScreenWidth() {

    switch(videoFormat) {

        case GAMEGEAR_NTSC:
            return 160;

        default:
            return getActiveDisplayWidth();
    }
}

uint16_t TMS9918A::getScreenHeight() {

    switch(videoFormat) {

        case GAMEGEAR_NTSC:
            return 144;

        default:
            return getActiveDisplayHeight();
    }
}

uint16_t TMS9918A::getScreenOffsetX() {

    if(videoFormat == GAMEGEAR_NTSC)
        return 48;

    return 0;
}

uint16_t TMS9918A::getScreenOffsetY() {

    if(videoFormat == GAMEGEAR_NTSC)
        return 24;

    return 0;
}

uint16_t TMS9918A::getNameTableBaseAddress() {

    if(getActiveDisplayHeight() != 192) {
        return ((reg[0x2] & 0b00001100) >> 2) * 0x1000 + 0x0700;
    }
    return ((reg[0x2] & 0b00001110) >> 1) * 0x0800;
}

uint16_t TMS9918A::getSpriteTableBaseAddress() {
    return ((reg[0x5] & 0b01111110) >> 1) * 0x0100;
}

uint8_t TMS9918A::readHCounter() {
    return hCounter >> 1;
}

uint8_t TMS9918A::readVCounter() {

    switch(videoFormat) {

        // NTSC
        case MASTERSYSTEM_NTSC:
        case GAMEGEAR_NTSC:
        {

            switch(getActiveDisplayHeight()) {

                case 192:
                {
                    if(vCounter >= 0xDB)
                        return 0xD5 + vCounter - 0xDB;

                    return vCounter;
                }

                case 224:
                {
                    if(vCounter >= 0xEB)
                        return 0xE5 + vCounter - 0xEB;

                    return vCounter;
                }

                case 240:
                {
                    if(vCounter >= 0x100)
                        return vCounter - 0x100;

                    return vCounter;
                }
            }
            break;
        }

        // PAL
        case MASTERSYSTEM_PAL:
        {

            switch(getActiveDisplayHeight()) {

                case 192:
                {
                    if(vCounter >= 0xF3)
                        return 0xBA + vCounter - 0xF3;

                    return vCounter;
                }

                case 224:
                {
                    if(vCounter >= 0x103)
                        return 0xCA + vCounter - 0x103;

                    if(vCounter >= 0x100)
                        return vCounter - 0x100;

                    return vCounter;
                }

                case 240:
                {
                    if(vCounter >= 0x10B)
                        return 0xD2 + vCounter - 0x10B;

                    if(vCounter >= 0x100)
                        return vCounter - 0x100;

                    return vCounter;
                }
            }
            break;
        }
    }
    return 0;
}

uint16_t TMS9918A::getHCounterLimit(){
    return 342;
}

uint16_t TMS9918A::getVCounterLimit(){

    // NTSC
    switch(videoFormat) {

        case MASTERSYSTEM_NTSC:
        case GAMEGEAR_NTSC:
        {
            return 262;
        }

        case MASTERSYSTEM_PAL:
        {
            return 313;
        }
    }
    return 0;
}