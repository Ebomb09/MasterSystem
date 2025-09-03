add_library(SN76489 STATIC
    SN76489.cpp
)

target_include_directories(SN76489 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)