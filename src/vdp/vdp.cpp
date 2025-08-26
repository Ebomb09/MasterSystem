#include "vdp.h"
#include <bitset>
#include <iostream>
#include <cstring>

vdp::vdp() {
    controlOffset = 0;

    requestFrameInterrupt = false;
    requestLineInterrupt = false;
}

void vdp::writeControlPort(uint8 data) {

    if(controlOffset == 0) {
        control = (control & 0xFF00) | data;
        controlOffset = 1;

    }else {
        control = (control & 0x00FF) | ((uint16)data << 8);
        controlOffset = 0;

        switch(getControlCode()) {

            // VRAM read
            case 0:
            {
                buffer = vram[getControlVRAMAddress()];
                incrementControlVRAMAddress();
                break;
            }

            // VRAM write
            case 1:
            {
                buffer = vram[getControlVRAMAddress()];
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
    uint8 res = buffer;

    buffer = vram[getControlVRAMAddress()];
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
            buffer = data;
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
    return (control & 0b1100000000000000) >> 14;
}

uint16 vdp::getControlVRAMAddress() {
    return (control & 0b0011111111111111);
}

void vdp::incrementControlVRAMAddress() {
    uint16 addr = getControlVRAMAddress();
    addr += 1;

    // Bound the address
    if(addr > 0x3FFF)
        addr = 0;

    control = (control & 0b1100000000000000) + addr;
}

uint8 vdp::getControlRegisterIndex() {
    return (control & 0b0000111100000000) >> 8;
}

uint8 vdp::getControlRegisterData() {
    return (control & 0b0000000011111111);
}

bool vdp::cycle() {
    bool clearVBlank = false;
    bool enableLineInterrupts   = reg[0x0] & 0b00010000;
    bool enableFrameInterrupts  = reg[0x1] & 0b00100000;

    hcounter ++;

    // Cleared HBlank
    if(hcounter >= 342) {
        hcounter = 0;
        vcounter ++;
    }

    // Cleared VBlank
    if(vcounter == 313) {
        clearVBlank = true;
        vcounter = 0;
    }

    // Check current line for any sprite collisions
    if(hcounter == 0) {
        scanSprites();
    }

    // Cleared Active Display
    if(vcounter == 192 && hcounter == 0) {
        status |= VBlank;
        if(enableFrameInterrupts) requestFrameInterrupt = true;
    }

    // Cleared LineCounter
    if(vcounter > 0 && reg[0xA] > 0 && vcounter % reg[0xA] == 0 && hcounter == 0) {
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
        if(y <= vcounter && vcounter < y+h) {
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