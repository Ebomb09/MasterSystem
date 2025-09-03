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
add_subdirectory(src/Z80)
add_subdirectory(src/TMS9918A)
add_subdirectory(src/SN76489)

# Emulation Frontend
add_executable(sms 
    src/main.cpp 
    src/sms.cpp
)
target_include_directories(sms PRIVATE src)
target_link_libraries(sms PRIVATE Z80 SN76489 TMS9918A SDL3-shared)

# Z80 CPU test
add_executable(test src/test.cpp)
target_include_directories(test PRIVATE src)
target_link_libraries(test PRIVATE Z80 nlohmann_json)
