#include "sms/sms.h"
#include <iostream>
#include <chrono>
#include <fstream>

#include "SDL3/SDL.h"

int main(int argc, char* argv[]) {

    // Emulator options
    int         optionRomPath = 0;
    int         optionScreenScale = 3;

    for(int i = 1; i < argc; i ++) {
        std::string option = argv[i];
        
        if(option == "--help") {
            

        }else if(option == "--scale" || option == "-s") {
            i ++;
            if(i < argc) optionScreenScale = std::max(1, std::stoi(argv[i]));

        }else {
            optionRomPath = i;
        }
    }

    // Initialize SDL3
    SDL_Window* window;
    SDL_Renderer* renderer;

    if(!SDL_CreateWindowAndRenderer("Master System", 256*optionScreenScale, 192*optionScreenScale, 0, &window, &renderer)) {
        std::cerr << "Error initializing the window and renderer\n";
        return 1;
    }
    SDL_SetRenderScale(renderer, optionScreenScale, optionScreenScale);

    // Initialize the emulator
    sms emu;

    if(!emu.loadRom(argv[optionRomPath])) {
        std::cerr << "Error loading the rom\n";
        return 1;
    }

    // Initialize the running loop
    bool run = true;
    toggleLog(false);

    while(run) {
        auto start = std::chrono::high_resolution_clock::now();

        // Read in SDL window inputs
        SDL_Event event;

        while(SDL_PollEvent(&event)) {

            if(event.type == SDL_EVENT_QUIT)
                run = false;

            if(event.type == SDL_EVENT_KEY_DOWN) {

                switch(event.key.scancode) {
                    case SDL_SCANCODE_UP: emu.joypad1 &= ~(1 << 0); break;
                    case SDL_SCANCODE_DOWN: emu.joypad1 &= ~(1 << 1); break;
                    case SDL_SCANCODE_LEFT: emu.joypad1 &= ~(1 << 2); break;
                    case SDL_SCANCODE_RIGHT: emu.joypad1 &= ~(1 << 3); break;
                    case SDL_SCANCODE_Z: emu.joypad1 &= ~(1 << 4); break;
                    case SDL_SCANCODE_X: emu.joypad1 &= ~(1 << 5); break;
                    case SDL_SCANCODE_SPACE: std::cout << std::hex << (int)emu.cpu.programCounter << "\n"; break;
                }
            }

            if(event.type == SDL_EVENT_KEY_UP) {

                switch(event.key.scancode) {
                    case SDL_SCANCODE_UP: emu.joypad1 |= (1 << 0); break;
                    case SDL_SCANCODE_DOWN: emu.joypad1 |= (1 << 1); break;
                    case SDL_SCANCODE_LEFT: emu.joypad1 |= (1 << 2); break;
                    case SDL_SCANCODE_RIGHT: emu.joypad1 |= (1 << 3); break;
                    case SDL_SCANCODE_Z: emu.joypad1 |= (1 << 4); break;
                    case SDL_SCANCODE_X: emu.joypad1 |= (1 << 5); break;
                }
            }
        }

        int time = emu.update(renderer);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        if(duration.count() < time) {
            int diff = time - duration.count();
            SDL_Delay(diff);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    std::clog << "Complete";

    return 0;
}