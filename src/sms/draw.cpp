#include "sms.h"

void sms::draw(SDL_Renderer* renderer) {
    bool enableDisplay = gpu.reg[0x1] & 0b01000000;

    if(!enableDisplay)
        return;

    // Select the Clear color from the sprite colors
    uint8 bgColor = gpu.reg[0x07] + 16;
    uint8 r = ((gpu.cram[bgColor] & 0b00000011) >> 0) * 85;
    uint8 g = ((gpu.cram[bgColor] & 0b00001100) >> 2) * 85;
    uint8 b = ((gpu.cram[bgColor] & 0b00110000) >> 4) * 85;
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderClear(renderer);

    if(enableDisplay) {
        drawTilemap(renderer);
        drawSprites(renderer);
        drawTilemap(renderer, true);
    }
    SDL_RenderPresent(renderer);
}

void sms::drawTile(SDL_Renderer* renderer, uint16 tileIndex, int x, int y, bool horizontalFlip, bool verticalFlip, bool spritePalette, bool doubleScale, bool tileWrap) {
    bool hideLeftMostPixels = (gpu.reg[0x0] & 0b00100000);
    
    uint16 addr = tileIndex * 32;

    for(int pixel_y = 0; pixel_y < 8; pixel_y ++) {
        uint8 byte[4];

        if(!verticalFlip) {
            byte[0] = gpu.vram[addr+pixel_y*4+0];
            byte[1] = gpu.vram[addr+pixel_y*4+1];
            byte[2] = gpu.vram[addr+pixel_y*4+2];
            byte[3] = gpu.vram[addr+pixel_y*4+3];
        }else {
            byte[0] = gpu.vram[addr+28-pixel_y*4+0];
            byte[1] = gpu.vram[addr+28-pixel_y*4+1];
            byte[2] = gpu.vram[addr+28-pixel_y*4+2];
            byte[3] = gpu.vram[addr+28-pixel_y*4+3];
        }

        if(horizontalFlip) {
            byte[0] = reverse(byte[0]);
            byte[1] = reverse(byte[1]);
            byte[2] = reverse(byte[2]);
            byte[3] = reverse(byte[3]);
        }

        for(int pixel_x = 0; pixel_x < 8; pixel_x ++) {
            uint8 paletteIndex = 0;

            if(byte[0] & (128 >> pixel_x))
                paletteIndex |= 0b0001;

            if(byte[1] & (128 >> pixel_x))
                paletteIndex |= 0b0010;

            if(byte[2] & (128 >> pixel_x))
                paletteIndex |= 0b0100;

            if(byte[3] & (128 >> pixel_x))
                paletteIndex |= 0b1000;

            // 0-15 BG, 16-31 Sprites
            if(spritePalette)
                paletteIndex += 16;

            if(paletteIndex == 0 || paletteIndex == 16)
                continue;

            uint8 r = ((gpu.cram[paletteIndex] & 0b00000011) >> 0) * 85;
            uint8 g = ((gpu.cram[paletteIndex] & 0b00001100) >> 2) * 85;
            uint8 b = ((gpu.cram[paletteIndex] & 0b00110000) >> 4) * 85;

            SDL_FRect rect;

            if(!doubleScale) {
                rect.x = x + pixel_x;
                rect.y = y + pixel_y;
                rect.w = 1;
                rect.h = 1;

            }else {
                rect.x = x + pixel_x * 2;
                rect.y = y + pixel_y * 2;
                rect.w = 2;
                rect.h = 2;   
            }

            if(tileWrap && rect.y >= 224)
                rect.y -= 224;

            if(hideLeftMostPixels && rect.x < 8)
                continue;

            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderRect(renderer, &rect);
        }
    }
}

void sms::drawTilemap(SDL_Renderer* renderer, bool drawPriority) {
    bool disableHorizontalScrollTopTwo      = (gpu.reg[0x0] & 0b01000000);
    bool disableVerticalScrollRightEight    = (gpu.reg[0x0] & 0b10000000);

    // Base address
    uint16 addr = ((gpu.reg[0x2] & 0b00001110) << 10);

    int x = 0, y = 0;

    uint8 scrollX = gpu.reg[0x8];
    uint8 scrollY = gpu.reg[0x9];

    for(int i = addr; i < 0x3EFF; i += 2) {
        uint16 entry = pairBytes(gpu.vram[i+1], gpu.vram[i]);

        uint16 tileIndex            = entry & 0b0000000111111111;
        bool horizontalFlip         = entry & 0b0000001000000000;
        bool verticalFlip           = entry & 0b0000010000000000;
        bool spritePalette          = entry & 0b0000100000000000;
        bool priority               = entry & 0b0001000000000000;

        int pos_x = x;
        int pos_y = y;

        if(!disableHorizontalScrollTopTwo || y >= 16) {
            pos_x += scrollX;

            if(pos_x > 256)
                pos_x -= 256;
        }

        if(!disableVerticalScrollRightEight || x <= 192) {
            pos_y -= scrollY;

            if(pos_y < 0)
                pos_y += 224;
        }

        if(priority == drawPriority)
            drawTile(renderer, tileIndex, pos_x, pos_y, horizontalFlip, verticalFlip, spritePalette, false, true);

        x += 8;
        if(x >= 256) {
            x = 0;
            y += 8;
        }
    }
}

void sms::drawSprites(SDL_Renderer* renderer) {
    bool enableDoubledSprites          = (gpu.reg[0x1] & 0b00000001);
    bool enableLargeSprites         = (gpu.reg[0x1] & 0b00000010);
    bool enable8thBitTileIndex      = (gpu.reg[0x6] & 0b00000100);
    bool enableShiftSpritesLeft     = (gpu.reg[0x0] & 0b00001000);

    // Base address
    uint16 addr = (gpu.reg[0x5] & 0b01111110) << 7;

    for(int i = i; i < 64; i ++) {
        uint8 y             = gpu.vram[addr+i];
        uint8 x             = gpu.vram[addr+0x80+i*2];
        uint16 tileIndex    = gpu.vram[addr+0x80+i*2+1];

        if(enable8thBitTileIndex)
            tileIndex |= 256;

        if(y == 0xD0) 
            break;

        if(enableShiftSpritesLeft)
            x -= 8;

        if(!enableLargeSprites) {
            drawTile(renderer, tileIndex, x, y, false, false, true, enableDoubledSprites);

        }else{
            drawTile(renderer, (tileIndex & ~1), x, y, false, false, true, enableDoubledSprites);         
            drawTile(renderer, (tileIndex & ~1)+1, x, y+8, false, false, true, enableDoubledSprites);
        }
    }
}