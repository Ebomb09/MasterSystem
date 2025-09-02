#include "vdp.h"

vdp::vdp() {
    controlOffset = 0;

    requestFrameInterrupt = false;
    requestLineInterrupt = false;

    for(int i = 0; i < 16; i ++)
        reg[i] = 0xFF;

    hCounter = 0;
    vCounter = 0;

    mode = 4;
}

void vdp::writeControlPort(uint8 data) {

    if(controlOffset == 0) {
        controlWord = (controlWord & 0xFF00) | data;
        controlOffset = 1;

    }else {
        controlWord = (controlWord & 0x00FF) | ((uint16)data << 8);
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

uint8 vdp::readControlPort() {
    controlOffset = 0;

    requestFrameInterrupt = false;
    requestLineInterrupt = false;

    // Return status and reset flags
    uint8 res = status;
    status = 0;
    return res;
}

uint8 vdp::readDataPort() {
    controlOffset = 0;

    // Return the value in the current buffer and then update, and increment
    uint8 res = readBuffer;

    readBuffer = vram[getControlVRAMAddress()];
    incrementControlVRAMAddress();
    
    return res;
}

void vdp::writeDataPort(uint8 data) {
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

uint8 vdp::getControlCode() {
    return (controlWord & 0b1100000000000000) >> 14;
}

uint16 vdp::getControlVRAMAddress() {
    return (controlWord & 0b0011111111111111);
}

void vdp::incrementControlVRAMAddress() {
    uint16 addr = getControlVRAMAddress();
    addr += 1;

    // Bound the address
    if(addr > 0x3FFF)
        addr = 0;

    controlWord = (controlWord & 0b1100000000000000) + addr;
}

uint8 vdp::getControlRegisterIndex() {
    return (controlWord & 0b0000111100000000) >> 8;
}

uint8 vdp::getControlRegisterData() {
    return (controlWord & 0b0000000011111111);
}

bool vdp::cycle() {
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

bool vdp::canSendInterrupt() {
    bool enableLineInterrupts   = reg[0x0] & 0b00010000;
    bool enableFrameInterrupts  = reg[0x1] & 0b00100000;

    return (enableFrameInterrupts && requestFrameInterrupt) || (enableLineInterrupts && requestLineInterrupt);
}

uint16 vdp::getActiveDisplayWidth() {
    return 256;
}

uint16 vdp::getActiveDisplayHeight() {

    if(reg[0x1] & 0b00001000) {
        return 240;

    }else if(reg[0x1] & 0b00010000) {
        return 224;
    }
    return 192;
}

uint16 vdp::getScreenWidth() {

    switch(deviceType) {

        case GAME_GEAR:
            return 160;

        default:
            return getActiveDisplayWidth();
    }
}

uint16 vdp::getScreenHeight() {

    switch(deviceType) {

        case GAME_GEAR:
            return 144;

        default:
            return getActiveDisplayHeight();
    }
}

uint16 vdp::getScreenOffsetX() {

    if(deviceType == GAME_GEAR)
        return 48;

    return 0;
}

uint16 vdp::getScreenOffsetY() {

    if(deviceType == GAME_GEAR)
        return 24;

    return 0;
}

uint16 vdp::getNameTableBaseAddress() {

    if(getActiveDisplayHeight() != 192) {
        return ((reg[0x2] & 0b00001100) >> 2) * 0x1000 + 0x0700;
    }
    return ((reg[0x2] & 0b00001110) >> 1) * 0x0800;
}

uint16 vdp::getSpriteTableBaseAddress() {
    return ((reg[0x5] & 0b01111110) >> 1) * 0x0100;
}

uint8 vdp::readHCounter() {
    return hCounter >> 1;
}

uint8 vdp::readVCounter() {

    switch(deviceType) {

        // NTSC
        case MASTER_SYSTEM_NTSC:
        case GAME_GEAR:
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
        case MASTER_SYSTEM_PAL:
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

uint16 vdp::getHCounterLimit(){
    return 342;
}

uint16 vdp::getVCounterLimit(){

    // NTSC
    switch(deviceType) {

        case MASTER_SYSTEM_NTSC:
        case GAME_GEAR:
        {
            return 262;
        }

        case MASTER_SYSTEM_PAL:
        {
            return 313;
        }
    }
    return 0;
}