#include "sms.h"

void sms::draw(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for(int x = 0; x < 256; x ++) {

        for(int y = 0; y < 192; y ++) {
            SDL_FRect rect {
                (float)x, (float)y, 1.f, 1.f
            };

            uint8 color = gpu.frameBuffer[x][y];
            uint8 r = ((color & 0b00000011) >> 0) * 85;
            uint8 g = ((color & 0b00001100) >> 2) * 85;
            uint8 b = ((color & 0b00110000) >> 4) * 85;

            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderRect(renderer, &rect);
        }
    }

    SDL_RenderPresent(renderer);
}