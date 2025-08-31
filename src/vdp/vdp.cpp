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

    switch(getDeviceType()) {

        case GAME_GEAR:
            return 160;

        default:
            return getActiveDisplayWidth();
    }
}

uint16 vdp::getScreenHeight() {

    switch(getDeviceType()) {

        case GAME_GEAR:
            return 144;

        default:
            return getActiveDisplayHeight();
    }
}

uint16 vdp::getScreenOffsetX() {

    if(getDeviceType() == GAME_GEAR)
        return 48;

    return 0;
}

uint16 vdp::getScreenOffsetY() {

    if(getDeviceType() == GAME_GEAR)
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

    switch(getDeviceType()) {

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
    switch(getDeviceType()) {

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

void vdp::drawScanLine() {
    bool enableDisplay = reg[0x1] & 0b01000000;

    // Render the background
    uint8 bgColor = reg[0x07] + 16;

    for(int x = 0; x < 256; x ++) 
        frameBuffer[x][vCounter] = getColor(bgColor);

    // Render the tilemap
    if(enableDisplay) {
        drawTilemap(false);
        drawSprites();
        drawTilemap(true);
    }
}

void vdp::drawTile(uint16 tileIndex, int x, int y, bool horizontalFlip, bool verticalFlip, bool spritePalette, bool doubleScale, bool tileWrap) {
    bool hideLeftMostPixels = (reg[0x0] & 0b00100000);
    
    uint16 addr = tileIndex * 32;
    uint16 tileMapHeight = (getActiveDisplayHeight() == 192) ? 8*28 : 8*32;

    uint8 size = (doubleScale) ? 16 : 8;

    for(int dot_y = 0; dot_y < size; dot_y ++) {
        int get_y = (doubleScale) ? dot_y / 2 : dot_y;

        if(y + dot_y != vCounter)
            continue;

        // Read in the 4 bytes that determine a dots palette index
        uint8 byte[4];

        if(!verticalFlip) {
            byte[0] = vram[addr+get_y*4+0];
            byte[1] = vram[addr+get_y*4+1];
            byte[2] = vram[addr+get_y*4+2];
            byte[3] = vram[addr+get_y*4+3];
        }else {
            byte[0] = vram[addr+28-get_y*4+0];
            byte[1] = vram[addr+28-get_y*4+1];
            byte[2] = vram[addr+28-get_y*4+2];
            byte[3] = vram[addr+28-get_y*4+3];
        }

        if(horizontalFlip) {
            byte[0] = reverse(byte[0]);
            byte[1] = reverse(byte[1]);
            byte[2] = reverse(byte[2]);
            byte[3] = reverse(byte[3]);
        }

        for(int dot_x = 0; dot_x < size; dot_x ++) {
            int get_x = (doubleScale) ? dot_x / 2 : dot_x;

            // Decode the palette index
            uint8 paletteIndex = 0;

            if(byte[0] & (128 >> get_x))
                paletteIndex |= 0b0001;

            if(byte[1] & (128 >> get_x))
                paletteIndex |= 0b0010;

            if(byte[2] & (128 >> get_x))
                paletteIndex |= 0b0100;

            if(byte[3] & (128 >> get_x))
                paletteIndex |= 0b1000;

            // 0-15 BG, 16-31 Sprites
            if(spritePalette)
                paletteIndex += 16;

            // Transparent palettes
            if(paletteIndex == 0 || paletteIndex == 16)
                continue;

            uint8 final_x = x + dot_x;
            uint8 final_y = y + dot_y;

            if(tileWrap && final_y >= tileMapHeight)
                final_y -= tileMapHeight;

            if(hideLeftMostPixels && final_x < 8)
                continue;

            frameBuffer[final_x][final_y] = getColor(paletteIndex);
        }
    }
}

void vdp::drawTilemap(bool drawPriority) {
    bool horizontalScrollLock       = (reg[0x0] & 0b01000000);
    bool verticalScrollLock         = (reg[0x0] & 0b10000000);

    // Base address
    uint16 addr = getNameTableBaseAddress();

    uint16 tileMapHeight = (getActiveDisplayHeight() == 192) ? 28*8 : 32*8;

    int x = 0, y = 0;

    uint8 scrollX = reg[0x8];
    uint8 scrollY = reg[0x9];

    for(int i = addr; i < 0x3F00; i += 2) {
        uint16 entry = pairBytes(vram[i+1], vram[i]);

        uint16 tileIndex            = entry & 0b0000000111111111;
        bool horizontalFlip         = entry & 0b0000001000000000;
        bool verticalFlip           = entry & 0b0000010000000000;
        bool spritePalette          = entry & 0b0000100000000000;
        bool priority               = entry & 0b0001000000000000;

        int pos_x = x;
        int pos_y = y;

        if(!horizontalScrollLock || y >= 16) {
            pos_x += scrollX;

            if(pos_x > 256)
                pos_x -= 256;
        }

        if(!verticalScrollLock || x < 248) {
            pos_y -= scrollY;

            if(pos_y < 0)
                pos_y += (getActiveDisplayHeight() == 192) ? 8*28 : 8*32;
        }

        if(pos_y <= vCounter && vCounter < pos_y+8) {

            if(priority == drawPriority)
                drawTile(tileIndex, pos_x, pos_y, horizontalFlip, verticalFlip, spritePalette, false, true);
        }

        // Check wrap around as well
        if(pos_y+8 >= tileMapHeight && pos_y-tileMapHeight <= vCounter && vCounter < pos_y-tileMapHeight+8) {

            if(priority == drawPriority)
                drawTile(tileIndex, pos_x, pos_y-tileMapHeight, horizontalFlip, verticalFlip, spritePalette, false, true);
        }

        x += 8;
        if(x >= 256) {
            x = 0;
            y += 8;
        }
    }
}

void vdp::drawSprites() {
    bool enableZoomedSprites        = (reg[0x1] & 0b00000001);
    bool enableStackedSprites       = (reg[0x1] & 0b00000010);
    bool enable8thBitTileIndex      = (reg[0x6] & 0b00000100);
    bool enableShiftSpritesLeft     = (reg[0x0] & 0b00001000);

    // Base address
    uint16 addr = getSpriteTableBaseAddress();

    // Sprite position tracker
    uint8 empty[256];
    std::memset(empty, true, 256);

    uint8 spriteBuffer = 0;

    // Sprite size
    uint8 size = (enableZoomedSprites) ? 16 : 8;
    uint8 combined_h = (enableStackedSprites) ? size * 2 : size;

    for(int i = 0; i < 64; i ++) {
        uint8 y             = vram[addr+i];
        uint8 x             = vram[addr+0x80+i*2];
        uint16 tileIndex    = vram[addr+0x80+i*2+1];

        if(enable8thBitTileIndex)
            tileIndex |= 256;

        // No more sprites, exit condition
        if(y == 0xD0) 
            break;

        if(enableShiftSpritesLeft)
            x -= 8;

        // Sprite occurs within vcounter
        if(y <= vCounter && vCounter < y+combined_h) {
            spriteBuffer ++;

            // Sprite overflow, exit condition
            if(spriteBuffer >= 8) {
                status |= SpriteOverflow;
                // Break for more accurate sprite flickering
                // break;
            }

            // Sprite collision
            if(status & SpriteCollision == 0) {

                for(int j = x; j < x+size; j ++) {

                    if(!empty[j]) {
                        status |= SpriteCollision;
                        break;
                    }
                    empty[j] = false;
                }
            }

            if(!enableStackedSprites) {
                drawTile(tileIndex, x, y, false, false, true, enableZoomedSprites);
    
            }else{
                drawTile((tileIndex & ~1), x, y, false, false, true, enableZoomedSprites);         
                drawTile((tileIndex & ~1)+1, x, y+size, false, false, true, enableZoomedSprites);
            }
        }
    }
}

int vdp::getColor(uint8 paletteIndex) {
    uint8 r, g, b;

    switch(getDeviceType()) {

        case MASTER_SYSTEM_NTSC:
        case MASTER_SYSTEM_PAL:
        {
            r = ((cram[paletteIndex] & 0b00000011) >> 0) * 85;
            g = ((cram[paletteIndex] & 0b00001100) >> 2) * 85;
            b = ((cram[paletteIndex] & 0b00110000) >> 4) * 85;
            break;
        }

        case GAME_GEAR:
        {
            r = ((cram[paletteIndex*2+0] & 0b00001111) >> 0) * 17;
            g = ((cram[paletteIndex*2+0] & 0b11110000) >> 4) * 17;
            b = ((cram[paletteIndex*2+1] & 0b00001111) >> 0) * 17;
            break;
        }
    }
    return (r << 0) | (g << 8) | (b << 16);
}