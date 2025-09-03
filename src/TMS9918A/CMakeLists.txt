add_library(TMS9918A STATIC
    TMS9918A.cpp
    draw.cpp
)

target_include_directories(TMS9918A PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)