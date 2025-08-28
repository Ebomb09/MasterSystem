#include "vdp.h"
#include <bitset>
#include <iostream>
#include <cstring>

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
            cram[getControlVRAMAddress() % 32] = data;
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

    uint16 screenWidth = getScreenWidth();
    uint16 screenHeight = getScreenHeight();
    uint16 hCounterLimit = getHCounterLimit();
    uint16 vCounterLimit = getVCounterLimit();

    hCounter ++;

    // Cleared HBlank
    if(hCounter >= hCounterLimit) {
        hCounter = 0;
        vCounter ++;
    }

    // Cleared VBlank
    if(vCounter >= vCounterLimit) {
        clearVBlank = true;
        vCounter = 0;
    }

    // Check current line for any sprite collisions
    if(hCounter == 0) {
        scanSprites();
    }

    // Cleared Active Display
    if(vCounter == screenHeight-1 && hCounter == screenWidth) {
        status |= VBlank;
        if(enableFrameInterrupts) requestFrameInterrupt = true;
    }

    // Cleared LineCounter
    if(reg[0xA] > 0 && (vCounter + 1) % reg[0xA] == 0 && vCounter < screenHeight && hCounter == screenWidth) {
        if(enableLineInterrupts) requestLineInterrupt = true;
    }
    return clearVBlank;
}

bool vdp::canSendInterrupt() {
    bool enableLineInterrupts   = reg[0x0] & 0b00010000;
    bool enableFrameInterrupts  = reg[0x1] & 0b00100000;

    return (enableFrameInterrupts && requestFrameInterrupt) || (enableLineInterrupts && requestLineInterrupt);
}

void vdp::scanSprites() {
    bool enableLargeSprites         = (reg[0x1] & 0b00000010);

    // Base address
    uint16 addr = (reg[0x5] & 0b01111110) << 7;

    // Sprite collision tracker
    uint8 empty[256];
    std::memset(empty, true, 256);
    uint8 spriteBuffer = 0;

    for(int i = i; i < 64; i ++) {
        uint8 y             = vram[addr+i];
        uint8 x             = vram[addr+0x80+i*2];

        if(y == 0xD0) 
            break;

        uint8 w = (enableLargeSprites) ? 16 : 8;
        uint8 h = (enableLargeSprites) ? 16 : 8;

        // Sprite occurs on vcounter
        if(y <= vCounter && vCounter < y+h) {
            spriteBuffer ++;

            // Sprite overflow
            if(spriteBuffer >= 8) {
                status |= SpriteOverflow;
                break;
            }

            // Sprite collision
            for(int j = x; j < x+w; j ++) {

                if(!empty[j]) {
                    status |= SpriteCollision;
                    break;
                }
                empty[j] = false;
            }
        }
    }
}

uint16 vdp::getScreenWidth() {
    return 256;
}

uint16 vdp::getScreenHeight() {

    if(reg[0x1] & 0b00001000) {
        return 240;

    }else if(reg[0x1] & 0b00010000) {
        return 224;
    }
    return 192;
}

uint16 vdp::getNameTableBaseAddress() {

    if(getScreenHeight() != 192) {
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

    switch(region & 1) {

        // NTSC
        case 0:
        {

            switch(getScreenHeight()) {

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
        case 1:
        {

            switch(getScreenHeight()) {

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
    if(region & 1 == 0) {
        return 262;

    // PAL
    }else {
        return 313;
    }
}
