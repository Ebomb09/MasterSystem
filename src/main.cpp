#include "sms.h"
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
    SDL_AudioStream* stream;

    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
        std::cerr << SDL_GetError();
        return 1;
    }

    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL, NULL, NULL);

    if(!stream) {
        std::cerr << SDL_GetError();
        return 1;
    }
    SDL_ResumeAudioStreamDevice(stream);

    if(!SDL_CreateWindowAndRenderer("Master System", 256*optionScreenScale, 192*optionScreenScale, 0, &window, &renderer)) {
        std::cerr << SDL_GetError();
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
    std::clog.setstate(std::iostream::failbit);

    while(run) {
        auto start = std::chrono::high_resolution_clock::now();

        // Read in SDL window inputs
        SDL_Event event;

        while(SDL_PollEvent(&event)) {

            if(event.type == SDL_EVENT_QUIT)
                run = false;

            if(event.type == SDL_EVENT_KEY_DOWN) {

                switch(event.key.scancode) {
                    case SDL_SCANCODE_UP:       emu.setJoyPadControl(sms::Joypad_A_Up, 0); break;
                    case SDL_SCANCODE_DOWN:     emu.setJoyPadControl(sms::Joypad_A_Down, 0); break;
                    case SDL_SCANCODE_LEFT:     emu.setJoyPadControl(sms::Joypad_A_Left, 0); break;
                    case SDL_SCANCODE_RIGHT:    emu.setJoyPadControl(sms::Joypad_A_Right, 0); break;
                    case SDL_SCANCODE_Z:        emu.setJoyPadControl(sms::Joypad_A_TL, 0); break;
                    case SDL_SCANCODE_X:        emu.setJoyPadControl(sms::Joypad_A_TR, 0); break;
                    case SDL_SCANCODE_SPACE:    std::cout << std::hex << (int)emu.cpu.programCounter << "\n"; break;
                    case SDL_SCANCODE_RETURN:   emu.setJoyPadControl(sms::Console_Reset, 0); break;
                }
            }

            if(event.type == SDL_EVENT_KEY_UP) {

                switch(event.key.scancode) {
                    case SDL_SCANCODE_UP:       emu.setJoyPadControl(sms::Joypad_A_Up, 1); break;
                    case SDL_SCANCODE_DOWN:     emu.setJoyPadControl(sms::Joypad_A_Down, 1); break;
                    case SDL_SCANCODE_LEFT:     emu.setJoyPadControl(sms::Joypad_A_Left, 1); break;
                    case SDL_SCANCODE_RIGHT:    emu.setJoyPadControl(sms::Joypad_A_Right, 1); break;
                    case SDL_SCANCODE_Z:        emu.setJoyPadControl(sms::Joypad_A_TL, 1); break;
                    case SDL_SCANCODE_X:        emu.setJoyPadControl(sms::Joypad_A_TR, 1); break;
                    case SDL_SCANCODE_RETURN:   emu.setJoyPadControl(sms::Console_Reset, 1); break;
                }
            }
        }

        // Resize the window based on the current device emulated
        int w, h;
        if(SDL_GetWindowSize(window, &w, &h)) {
            w /= optionScreenScale;
            h /= optionScreenScale;

            if(w != emu.gpu.getScreenWidth() || h != emu.gpu.getScreenHeight()) {
                SDL_SetWindowSize(window, emu.gpu.getScreenWidth() * optionScreenScale, emu.gpu.getScreenHeight() * optionScreenScale);
                SDL_Rect viewport {
                    -emu.gpu.getScreenOffsetX(),
                    -emu.gpu.getScreenOffsetY(),
                    emu.gpu.getScreenWidth() + emu.gpu.getScreenOffsetX(),
                    emu.gpu.getScreenHeight() + emu.gpu.getScreenOffsetY()
                };
                SDL_SetRenderViewport(renderer, &viewport);
            }
        }

        // Emulate a frame and sync with the time a real system would take
        int time = emu.update(renderer, stream);

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