cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    sdl3
    GIT_REPOSITORY git@github.com:libsdl-org/SDL.git
    GIT_TAG release-3.2.x
)

FetchContent_Declare(
    json
    GIT_REPOSITORY git@github.com:nlohmann/json.git
    GIT_TAG master
)

FetchContent_MakeAvailable(sdl3 json)

project(MasterSystemInterpreter)

set(MasterSystemSource
    src/utilities.cpp

    # Z80 Processor Sources
    src/z80/z80.cpp
    src/z80/mnemonics.cpp
    src/z80/instructions/8bit_load_group.cpp
    src/z80/instructions/16bit_load_group.cpp
    src/z80/instructions/exchange_search_group.cpp
    src/z80/instructions/general_arithmetic_group.cpp
    src/z80/instructions/8bit_arithmetic_group.cpp
    src/z80/instructions/16bit_arithmetic_group.cpp
    src/z80/instructions/rotate_shift_group.cpp
    src/z80/instructions/bit_set_reset_test_group.cpp
    src/z80/instructions/jump_group.cpp
    src/z80/instructions/call_return_group.cpp
    src/z80/instructions/input_output_group.cpp

    # Sega VDP sources
    src/vdp/vdp.cpp
    src/vdp/draw.cpp

    # Sega PSG sources
    src/psg/psg.cpp

    # Emulator sources
    src/sms/sms.cpp
)

add_executable(sms src/main.cpp ${MasterSystemSource})
target_include_directories(sms PRIVATE src)
target_link_libraries(sms PRIVATE SDL3-shared)

add_executable(test src/test.cpp ${MasterSystemSource})
target_include_directories(test PRIVATE src)
target_link_libraries(test PRIVATE SDL3-shared nlohmann_json)
