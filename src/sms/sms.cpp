#include "sms.h"
#include <iomanip>
#include <stdexcept>
#include <fstream>
#include <functional>

sms::sms() {
    using namespace std::placeholders;

    cpu.port_read = std::bind(sms::port_read, this, _1);
    cpu.port_write = std::bind(sms::port_write, this, _1, _2);
    cpu.mapper_read = std::bind(sms::mapper_read, this, _1);
    cpu.mapper_write = std::bind(sms::mapper_write, this, _1, _2);

    // Initialize the slot indices
    ram[0x1FFC] = 0;
    ram[0x1FFD] = 0;
    ram[0x1FFE] = 1;
    ram[0x1FFF] = 2;

    // Joypad inputs are pull down
    joypad1 = 0xFF;
    joypad2 = 0xFF;
    joypadControl = 0xFF;
    memoryControl = 0xFF;
}

sms::~sms() {
    
    if(rom)
        delete [] rom;
}

bool sms::loadRom(std::string romPath) {
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

int sms::update(SDL_Renderer* renderer) {
    bool clearVBlank = false;
    int totalClock = 0;

    while(!clearVBlank) {

        // Each GPU cycle is 3 CPU cycles
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

        for(int i = 0; i < clock * 3; i ++) {

            if(gpu.cycle())
                clearVBlank = true;

            if(gpu.vcounter == 192 && gpu.hcounter == 0)
                draw(renderer);
        }

        // Dispatch interrupts from gpu to cpu
        if(gpu.canSendInterrupt()) 
            cpu.signalINT();

        totalClock += clock;
    }

    // Return the theoretical time(ms) to clear a VBlank 
    return totalClock * 15 * 1000 / 53693100;
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

    if(addr >= 0x00 && addr <= 0x3E && addr % 2 == 0) {
        return memoryControl;


    }else if(addr >= 0x01 && addr <= 0x3F && addr % 2 == 1) {
        return joypadControl;


    }else if(addr >= 0x40 && addr <= 0x7E && addr % 2 == 0) {
        
        if(gpu.vcounter > 192) {
            return gpu.reg[0xA] + gpu.vcounter - 193;
        }else {
            return gpu.vcounter;
        }


    }else if(addr >= 0x41 && addr <= 0x7F && addr % 2 == 1) {
        return gpu.hcounter >> 1;


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

    // SDSC, debug console
    if(addr == 0xFD) 
        std::cout << (char)data;

    if(addr >= 0x00 && addr <= 0x3E && addr % 2 == 0) {
        memoryControl = data;


    }else if(addr >= 0x01 && addr <= 0x3F && addr % 2 == 1) {
        joypadControl = data;

    }else if(addr >= 0x40 && addr <= 0x7F) {
        audio.write(data);


    }else if(addr >= 0x80 && addr <= 0xBE && addr % 2 == 0) {
        gpu.writeDataPort(data);

        
    }else if(addr >= 0x81 && addr <= 0xBF && addr % 2 == 1) {
        gpu.writeControlPort(data);

    }
}