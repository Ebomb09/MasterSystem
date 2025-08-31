# Master System Emulator
My implementation of the Z80 instruction set, SN76489 sound generator, and TMS9918A display controller, for use to emulate Sega Master System, and Sega Game Gear software.

# Technologies
* [SDL3](https://github.com/libsdl-org/SDL)
* [json](https://github.com/nlohmann/json)

# Resources
* [SMS Power!](https://www.smspower.org)
* [Z80 CPU Manual](https://www.zilog.com/docs/z80/um0080.pdf)
* [SN76489 Manual](https://map.grauw.nl/resources/sound/texas_instruments_sn76489an.pdf)

# Building
Requires a copy of SDL3 and json in `ext/`.

1. `mkdir build`
2. `cd build`
3. `cmake ../`
4. `make`

# Usage
```
sms [options] program
        --scale <display scale> Sets the scaling of the 256x192 display
```

### Controls:

Player1: 
- Directional Pad: Arrow Keys
- Button Left: Z
- Button Right: X
- Reset: Enter
