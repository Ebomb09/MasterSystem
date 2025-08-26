cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

    # Sega PSG sources
    src/psg/psg.cpp

    # Emulator sources
    src/sms/sms.cpp
    src/sms/draw.cpp
)

set(LINK_DIRS ext/SDL3-3.2.10/x86_64-w64-mingw32/lib)
set(LINK_LIBS SDL3)
set(INCLUDE_DIRS src ext/SDL3-3.2.10/x86_64-w64-mingw32/include ext/nlohmann/include)

add_executable(sms src/main.cpp ${MasterSystemSource})
target_include_directories(sms PUBLIC ${INCLUDE_DIRS})
target_link_directories(sms PUBLIC ${LINK_DIRS})
target_link_libraries(sms PUBLIC ${LINK_LIBS})

add_executable(test src/test.cpp ${MasterSystemSource})
target_include_directories(test PUBLIC ${INCLUDE_DIRS})
target_link_directories(test PUBLIC ${LINK_DIRS})
target_link_libraries(test PUBLIC ${LINK_LIBS})
