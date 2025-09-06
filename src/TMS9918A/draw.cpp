#include "TMS9918A.h"
#include "utilities.h"
#include <cstring>
#include <iostream>

void TMS9918A::drawScanLine() {

    // Reset line
    uint8_t bgPalette = reg[0x7] + 16;

    if(vCounter < getActiveDisplayHeight()) {

        for(int x = 0; x < 256; x ++) {
            frameBuffer[x + vCounter * 256] = getColor(bgPalette);
            depthBuffer[x + vCounter * 256] = CLEAR;
        }
    }

    drawSprites();
    drawTilemap();
}

bool TMS9918A::drawPixel(int x, int y, int color, int depth, bool force) {
    bool enableDisplay = reg[0x1] & 0b01000000;

    int index = x + y * 256;

    if(index >= 0 && index < 256 * 313) {

        if(depthBuffer[index] < depth) {

            if(enableDisplay || force) {
                frameBuffer[index] = color;
                depthBuffer[index] = depth;
            }
            return true;

        // Set sprite collision
        }else if(depthBuffer[index] == SPRITE && depth == SPRITE && (status & SpriteOverflow) == 0 && x < 255) {
            status |= SpriteCollision;
        }
    }
    return false;
}

bool TMS9918A::drawTile(int tileIndex, int x, int y, int drawMode, bool doubleScale, bool horizontalFlip, bool verticalFlip) {
    bool hideLeftMostPixels = (reg[0x0] & 0b00100000);

    uint16_t addr = tileIndex * 32;
    uint16_t tileMapHeight = (getActiveDisplayHeight() == 192) ? 28*8 : 32*8;

    uint8_t size = (doubleScale) ? 16 : 8;

    bool res = false;

    for(int dot_y = 0; dot_y < size; dot_y ++) {
        int get_y = (doubleScale) ? dot_y / 2 : dot_y;
        int draw_y = y + dot_y;

        // Wrap tile pixels around tile map
        if(drawMode != SPRITE && draw_y >= tileMapHeight)
            draw_y -= tileMapHeight;

        if(draw_y != vCounter)
            continue;

        // Read in the 4 bytes that determine a dots palette index
        uint8_t byte[4];

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
            int draw_x = x + dot_x;

            // Decode the palette index
            uint8_t paletteIndex = 0;

            if(byte[0] & (128 >> get_x))
                paletteIndex |= 0b0001;

            if(byte[1] & (128 >> get_x))
                paletteIndex |= 0b0010;

            if(byte[2] & (128 >> get_x))
                paletteIndex |= 0b0100;

            if(byte[3] & (128 >> get_x))
                paletteIndex |= 0b1000;


            // Transparent palettes
            int currentDrawMode = drawMode;

            if(paletteIndex == 0) {

                if(drawMode == SPRITE) {
                    continue;

                }else if(drawMode == TILE_PRIORITY || drawMode == TILE_ALT_PRIORITY) {
                    currentDrawMode -= 3;
                }
            }

            // Secondary palette
            if(drawMode == SPRITE || drawMode == TILE_ALT || drawMode == TILE_ALT_PRIORITY) {
                paletteIndex += 16;
            }

            if(hideLeftMostPixels && draw_x < 8)
                continue;

            if(drawPixel(draw_x, draw_y, getColor(paletteIndex), currentDrawMode)) {
                res = true;
            }
        }
    }
    return res;
}

void TMS9918A::drawTilemap() {
    bool horizontalScrollLock       = (reg[0x0] & 0b01000000);
    bool verticalScrollLock         = (reg[0x0] & 0b10000000);

    uint16_t tileMapHeight = (getActiveDisplayHeight() == 192) ? 28*8 : 32*8;

    uint16_t addr = getNameTableBaseAddress();

    int x = 0, y = 0;

    uint8_t scrollX = reg[0x8];
    uint8_t scrollY = reg[0x9];

    for(int i = 0; i < getNameTableSize(); i ++) {
        uint16_t entry = pairBytes(vram[addr+i*2+1], vram[addr+i*2+0]);

        uint16_t tileIndex          = entry & 0b0000000111111111;
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
                pos_y += tileMapHeight;
        }

        bool fallsOnLine = 
            (pos_y <= vCounter && vCounter < pos_y+8) || 
            (pos_y-tileMapHeight <= vCounter && vCounter < pos_y-tileMapHeight+8);

        if(fallsOnLine) {
            int drawMode = (spritePalette) ? TILE_ALT : TILE;

            if(priority)
                drawMode += 3;

            drawTile(tileIndex, pos_x, pos_y, drawMode, false, horizontalFlip, verticalFlip);
        }

        x += 8;
        if(x >= 256) {
            x = 0;
            y += 8;
        }
    }
}

void TMS9918A::drawSprites() {
    bool enableDisplay = reg[0x1] & 0b01000000;
    bool enableZoomedSprites        = (reg[0x1] & 0b00000001);
    bool enableStackedSprites       = (reg[0x1] & 0b00000010);
    bool enable8thBitTileIndex      = (reg[0x6] & 0b00000100);
    bool enableShiftSpritesLeft     = (reg[0x0] & 0b00001000);

    // Base address
    uint16_t addr = getSpriteTableBaseAddress();

    uint8_t spriteBuffer = 0;

    // Sprite size
    uint8_t size = (enableZoomedSprites) ? 16 : 8;
    uint8_t combined_h = (enableStackedSprites) ? size * 2 : size;

    for(int i = 0; i < getSpriteTableSize(); i ++) {
        int y               = vram[addr+i];
        int x               = vram[addr+0x80+i*2];
        uint16_t tileIndex  = vram[addr+0x80+i*2+1];

        if(enable8thBitTileIndex)
            tileIndex |= 256;

        // No more sprites, exit condition
        if(y == 0xD0) 
            break;

        if(enableShiftSpritesLeft)
            x -= 8;

        // Note: Sprites are drawn one pixel lower
        y += 1;

        bool drawn = false;

        if(!enableStackedSprites) {
            drawn = drawTile(tileIndex, x, y, SPRITE, enableZoomedSprites);

        }else{
            drawn = drawTile((tileIndex & ~1), x, y, SPRITE, enableZoomedSprites) ||
                    drawTile((tileIndex & ~1)+1, x, y+size, SPRITE, enableZoomedSprites);
        }

        if(drawn) {

            if(y < getActiveDisplayHeight())
                spriteBuffer ++;

            // Sprite overflow, exit condition
            if(spriteBuffer >= 8) {
                status |= SpriteOverflow;                
                break;
            }
        }
    }
}

int TMS9918A::getColor(int paletteIndex) {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    switch(videoFormat) {

        case MASTERSYSTEM_NTSC:
        case MASTERSYSTEM_PAL:
        {
            r = ((cram[paletteIndex] & 0b00000011) >> 0) * 85;
            g = ((cram[paletteIndex] & 0b00001100) >> 2) * 85;
            b = ((cram[paletteIndex] & 0b00110000) >> 4) * 85;
            break;
        }

        case GAMEGEAR_NTSC:
        {
            r = ((cram[paletteIndex*2+0] & 0b00001111) >> 0) * 17;
            g = ((cram[paletteIndex*2+0] & 0b11110000) >> 4) * 17;
            b = ((cram[paletteIndex*2+1] & 0b00001111) >> 0) * 17;
            break;
        }
    }
    return (r << 24) | (g << 16) | (b << 8) | (a << 0);
}