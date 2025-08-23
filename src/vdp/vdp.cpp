#include "vdp.h"
#include <bitset>

vdp::vdp() {
    controlOffset = 0;
}

void vdp::writeControlPort(uint8 data) {

    if(controlOffset == 0) {
        control = (control & 0xFF00) | data;
        controlOffset = 1;

    }else {
        control = (control & 0x00FF) | ((uint16)data << 8);
        controlOffset = 0;

        switch(getControlCode()) {

            // VRAM read
            case 0:
            {
                buffer = vram[getControlVRAMAddress()];
                incrementControlVRAMAddress();
                break;
            }

            // VRAM write
            case 1:
            {
                buffer = vram[getControlVRAMAddress()];
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

    // Reset interrupts
    requestInterrupt = false;

    // Return status and reset flags
    uint8 res = status;
    status = 0;
    return res;
}

uint8 vdp::readDataPort() {
    controlOffset = 0;

    // Return the value in the current buffer and then update, and increment
    uint8 res = buffer;

    buffer = vram[getControlVRAMAddress()];
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
            buffer = data;
            incrementControlVRAMAddress();
            break;
        }

        case 3:
        {
            cram[getControlVRAMAddress() % 32] = data;
            incrementControlVRAMAddress();
            break;
        }
    }
}

uint8 vdp::getControlCode() {
    return (control & 0b1100000000000000) >> 14;
}

uint16 vdp::getControlVRAMAddress() {
    return (control & 0b0011111111111111);
}

void vdp::incrementControlVRAMAddress() {
    uint16 addr = getControlVRAMAddress();
    addr += 1;

    // Bound the address
    if(addr > 0x3FFF)
        addr = 0;

    control = (control & 0b1100000000000000) + addr;
}

uint8 vdp::getControlRegisterIndex() {
    return (control & 0b0000111100000000) >> 8;
}

uint8 vdp::getControlRegisterData() {
    return (control & 0b0000000011111111);
}

bool vdp::cycle() {
    bool clearVBlank = false;
    bool enableLineInterrupts   = reg[0x0] & 0b00010000;
    bool enableFrameInterrupts  = reg[0x1] & 0b00100000;

    hcounter ++;

    if(hcounter >= 342) {
        hcounter = 0;
        vcounter ++;
    }

    if(vcounter == 313) {
        clearVBlank = true;
        vcounter = 0;
    }

    if(enableFrameInterrupts) {

        if(vcounter == 192 && hcounter == 0) {
            status |= VBlank;
        }

        if(status & VBlank) {
            if(enableFrameInterrupts) requestInterrupt = true;
        }
    }

    if(enableLineInterrupts) {

        if(hcounter == reg[0xA]) {
            if(enableLineInterrupts) requestInterrupt = true;
        }
    }
    return clearVBlank;
}