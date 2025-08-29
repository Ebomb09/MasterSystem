#include "sms.h"
#include <iomanip>
#include <stdexcept>
#include <fstream>
#include <functional>
#include <cmath>

sms::sms() {
    using namespace std::placeholders;

    cpu.port_read = std::bind(sms::port_read, this, _1);
    cpu.port_write = std::bind(sms::port_write, this, _1, _2);
    cpu.mapper_read = std::bind(sms::mapper_read, this, _1);
    cpu.mapper_write = std::bind(sms::mapper_write, this, _1, _2);

    gpu.getDeviceType = std::bind(sms::getDeviceType, this);

    // Initialize the slot indices
    ram[0x1FFC] = 0;
    ram[0x1FFD] = 0;
    ram[0x1FFE] = 1;
    ram[0x1FFF] = 2;

    // Joypad inputs are pull down
    joypad1 = 0xFF;
    joypad2 = 0xFF;
    joypadStart = 0xFF;
}

sms::~sms() {
    
    if(rom)
        delete [] rom;
}

bool sms::loadRom(std::string romPath) {

    // Load the contents of rom
    romSize = 0;

    std::fstream file(romPath, std::fstream::binary | std::fstream::in);

    if(!file.good()) {
        file.close();
        return false;
    }
    file.seekg(0, file.end);
    romSize = file.tellg();
    file.seekg(0, file.beg);

    rom = new uint8[romSize];
    file.read((char*)rom, romSize);

    file.close();
    return true;
}

int sms::update(SDL_Renderer* renderer, SDL_AudioStream* stream) {
    bool clearVBlank = false;
    int totalClock = 0;

    while(!clearVBlank) {
        int clock = cpu.cycle();

        // Invalid opcode found
        if(clock <= 0) {

            uint8 byte[4] {
                cpu.mapper_read(cpu.programCounter),
                cpu.mapper_read(cpu.programCounter+1),
                cpu.mapper_read(cpu.programCounter+2),
                cpu.mapper_read(cpu.programCounter+3)
            };
            std::cout << std::hex << (int)cpu.programCounter << ": " << (int)byte[0] << " " << (int)byte[1] << " " << (int)byte[2] << " " << (int)byte[3] << "\n";
            throw std::runtime_error("INVALID OPCODE");
        } 

        // Cycle the gpu device
        for(int i = 0; i < clock * 3 / 2; i ++) {

            if(gpu.cycle())
                clearVBlank = true;

            if(gpu.vCounter == 192 && gpu.hCounter == 0)
                draw(renderer);
        }

        // Dispatch interrupts from gpu to cpu
        if(gpu.canSendInterrupt()) 
            cpu.signalINT();

        totalClock += clock;
    }

    // Generate the sound waves
    std::vector<float> samples(totalClock / 15);
    for(int i = 0; i < samples.size(); i ++ ) {
        audio.cycle();
        samples[i] = audio.getSample();
    }

    // Calculate the theoretical time(ms) to clear a VBlank 
    int time = totalClock * 15 * 1000 / getMasterClock();

    // Using time to determine format of the sample stream
    if(time > 0) {
        SDL_AudioSpec spec;
        spec.channels = 1;
        spec.format = SDL_AUDIO_F32;
        spec.freq = 1000 * samples.size() / time;
        SDL_SetAudioStreamFormat(stream, &spec, NULL);
        SDL_PutAudioStreamData(stream, samples.data(), samples.size() * sizeof(float));
        SDL_FlushAudioStream(stream);
    }

    return time;
}

uint8 sms::mapper_read(uint16 addr) {
    uint8 options = ram[0x1FFC];
    uint8 slot0 = ram[0x1FFD];
    uint8 slot1 = ram[0x1FFE];
    uint8 slot2 = ram[0x1FFF];

    enum {
        BankShift               = 0b00000011,
        RAMBankSelect           = 0b00000100,
        RAMEnableSlot2          = 0b00001000,
        RAMEnableSlotMirror     = 0b00010000,
        ROMEnableWrite          = 0b10000000
    };

    // First 1kb is always the first 1kb of rom
    if(0x0000 <= addr && addr <= 0x03ff) {
        return rom[addr];

    // Slot0 of 16kb rom
    }else if(0x0400 <= addr && addr <= 0x3fff) {
        return rom[(slot0 * 16*1024 + addr) % romSize];

    // Slot1 of 16kb rom
    }else if(0x4000 <= addr && addr <= 0x7fff) {
        return rom[(slot1 * 16*1024 + addr - 0x4000) % romSize]; 
    
    // Slot2 of 16kb rom/ram
    }else if(0x8000 <= addr && addr <= 0xbfff) {
        
        if(options & RAMEnableSlot2)
            return sram[(options & RAMBankSelect) >> 2][addr - 0x8000];

        return rom[(slot2 * 16*1024 + addr - 0x8000) % romSize]; 
    
    // System RAM
    }else if(0xc000 <= addr && addr <= 0xdfff) {
        return ram[addr - 0xc000];

    // System mirrored RAM
    }else {
        return ram[addr - 0xe000];
    }
}

void sms::mapper_write(uint16 addr, uint8 data) {
    uint8 options = ram[0x1FFC];
    uint8 slot0 = ram[0x1FFD];
    uint8 slot1 = ram[0x1FFE];
    uint8 slot2 = ram[0x1FFF];

    enum {
        BankShift               = 0b00000011,
        RAMBankSelect           = 0b00000100,
        RAMEnableSlot2          = 0b00001000,
        RAMEnableSlotMirror     = 0b00010000,
        ROMEnableWrite          = 0b10000000
    };

    // First 1kb is always the first 1kb of rom
    if(0x0000 <= addr && addr <= 0x03ff) {
        //cannot write to rom

    // Slot0 of 16kb rom
    }else if(0x0400 <= addr && addr <= 0x3fff) {
        // Cannot write to rom

    // Slot1 of 16kb rom
    }else if(0x4000 <= addr && addr <= 0x7fff) {
        // Cannot write to rom 
    
    // Slot2 of 16kb rom/ram
    }else if(0x8000 <= addr && addr <= 0xbfff) {
        
        if(options & RAMEnableSlot2) 
            sram[(options & RAMBankSelect) >> 2][addr - 0x8000] = data;
    
    // System RAM
    }else if(0xc000 <= addr && addr <= 0xdfff) {
        ram[addr - 0xc000] = data;

    // System mirrored RAM
    }else {
        ram[addr - 0xe000] = data;
    }
}

uint8 sms::port_read(uint16 addr) {
    addr %= 256;

    if(getDeviceType() == GAME_GEAR && addr == 0x00) {
        return joypadStart;

    }else if(addr >= 0x00 && addr <= 0x3E && addr % 2 == 0) {
        // Memory control, unneeded
        return 0xFF;


    }else if(addr >= 0x01 && addr <= 0x3F && addr % 2 == 1) {
        // I/O Port Control, unneeded
        return 0xFF;


    }else if(addr >= 0x40 && addr <= 0x7E && addr % 2 == 0) {
        return gpu.readVCounter();


    }else if(addr >= 0x41 && addr <= 0x7F && addr % 2 == 1) {
        return gpu.readHCounter();


    }else if(addr >= 0x80 && addr <= 0xBE && addr % 2 == 0) {
        return gpu.readDataPort();


    }else if(addr >= 0x81 && addr <= 0xBF && addr % 2 == 1) {
        return gpu.readControlPort();


    }else if(addr >= 0xC0 && addr <= 0xFE && addr % 2 == 0) {
        return joypad1;


    }else if(addr >= 0xC1 && addr <= 0xFF && addr % 2 == 1) {
        return joypad2;

    }
    return 0;
}

void sms::port_write(uint16 addr, uint8 data) {
    addr %= 256;

    if(addr == 0xFD) {
        // SDSC, debug console
        std::cout << (char)data;


    }else if(addr >= 0x40 && addr <= 0x7F) {
        audio.write(data);


    }else if(addr >= 0x80 && addr <= 0xBE && addr % 2 == 0) {
        gpu.writeDataPort(data);

        
    }else if(addr >= 0x81 && addr <= 0xBF && addr % 2 == 1) {
        gpu.writeControlPort(data);

    }
}

int sms::getDeviceType() {

    switch((rom[0x7fff] & 0b11110000) >> 4) {
        case 0x3: return MASTER_SYSTEM_NTSC;
        case 0x4: return MASTER_SYSTEM_PAL;
        case 0x5: return GAME_GEAR;
        case 0x6: return GAME_GEAR;
        case 0x7: return GAME_GEAR;
    }
    return 0;
}

int sms::getMasterClock() {

    switch(getDeviceType()) {

        case MASTER_SYSTEM_NTSC: 
        case GAME_GEAR:
            return 53693100;

        case MASTER_SYSTEM_PAL:
            return 53203400;
    }
    return 1;
}

void sms::setJoyPadControl(uint8 control, bool val) {
    uint8* ptr = NULL;
    uint8 bit = 0;

    switch(control) {

        case Joypad_B_Down:     bit = 1 << 7; ptr = &joypad1; break;
        case Joypad_B_Up:       bit = 1 << 6; ptr = &joypad1; break;
        case Joypad_A_TR:       bit = 1 << 5; ptr = &joypad1; break;
        case Joypad_A_TL:       bit = 1 << 4; ptr = &joypad1; break;
        case Joypad_A_Right:    bit = 1 << 3; ptr = &joypad1; break;
        case Joypad_A_Left:     bit = 1 << 2; ptr = &joypad1; break;
        case Joypad_A_Down:     bit = 1 << 1; ptr = &joypad1; break;
        case Joypad_A_Up:       bit = 1 << 0; ptr = &joypad1; break;

        case Joypad_B_TH:       bit = 1 << 7; ptr = &joypad2; break;
        case Joypad_A_TH:       bit = 1 << 6; ptr = &joypad2; break;
        case Joypad_B_TR:       bit = 1 << 3; ptr = &joypad2; break;
        case Joypad_B_TL:       bit = 1 << 2; ptr = &joypad2; break;
        case Joypad_B_Right:    bit = 1 << 1; ptr = &joypad2; break;
        case Joypad_B_Left:     bit = 1 << 0; ptr = &joypad2; break;

        case Console_Reset:     
        {
            if(getDeviceType() == GAME_GEAR){
                bit = 1 << 7;
                ptr = &joypadStart;

            }else {
                if(!val) cpu.signalNMI();
            }
        }
    }

    // Toggle the bit and set the value
    if(ptr) {
        (*ptr) &= ~bit;

        if(val)
            (*ptr) |= bit;
    }
}