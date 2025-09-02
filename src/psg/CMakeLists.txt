add_library(psg STATIC
    psg.cpp
)

target_include_directories(psg PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)