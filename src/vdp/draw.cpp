#include "vdp.h"
#include <cstring>

void vdp::drawScanLine() {
    bool enableDisplay = reg[0x1] & 0b01000000;

    // Render the background
    uint8 bgColor = reg[0x07] + 16;

    for(int x = 0; x < 256; x ++) 
        frameBuffer[x + vCounter * 256] = getColor(bgColor);

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

            frameBuffer[final_x + final_y * 256] = getColor(paletteIndex);
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
            if((status & SpriteCollision) == 0) {

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
    uint8 r = 0;
    uint8 g = 0;
    uint8 b = 0;
    uint8 a = 255;

    switch(deviceType) {

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
    return (r << 24) | (g << 16) | (b << 8) | (a << 0);
}