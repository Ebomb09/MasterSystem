cmake_minimum_required(VERSION 3.27)

project(MasterSystemInterpreter)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Retrieve Dependencies
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

# Emulated Processors
add_subdirectory(src/z80)
add_subdirectory(src/vdp)
add_subdirectory(src/psg)

# Emulation Frontend
add_executable(sms 
    src/main.cpp 
    src/sms/sms.cpp
)
target_include_directories(sms PRIVATE src)
target_link_libraries(sms PRIVATE z80 psg vdp SDL3-shared)

# Z80 CPU test
add_executable(test src/test.cpp)
target_include_directories(test PRIVATE src)
target_link_libraries(test PRIVATE z80 SDL3-shared nlohmann_json)
