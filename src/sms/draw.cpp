#include "sms.h"

void sms::draw(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    int limitX = gpu.getScreenWidth() + gpu.getScreenOffsetX();
    int limitY = gpu.getScreenHeight() + gpu.getScreenOffsetY();

    for(int x = gpu.getScreenOffsetX(); x < limitX; x ++) {
        for(int y = gpu.getScreenOffsetY(); y < limitY; y ++) {
            SDL_FRect rect {
                (float)x, (float)y, 1.f, 1.f
            };

            int color = gpu.frameBuffer[x][y];
            uint8 r = color >> 0;
            uint8 g = color >> 8;
            uint8 b = color >> 16;

            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderRect(renderer, &rect);
        }
    }

    SDL_RenderPresent(renderer);
}