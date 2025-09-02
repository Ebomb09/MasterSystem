add_library(vdp STATIC
    vdp.cpp
    draw.cpp

    ../common/utilities.cpp
)

target_include_directories(vdp PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)