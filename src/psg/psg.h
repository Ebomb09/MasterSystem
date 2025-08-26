#ifndef SEGA_PSG_H
#define SEGA_PSG_H

#include "utilities.h"

struct psg {

    uint8 reg[4];

    uint8 latchChannel;
    uint8 latchType;

    void write(uint8 byte);
};

#endif