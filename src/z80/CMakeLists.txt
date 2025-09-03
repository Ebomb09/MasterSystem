add_library(Z80 STATIC 
    Z80.cpp
    mnemonics.cpp
    instructions/8bit_load_group.cpp
    instructions/16bit_load_group.cpp
    instructions/exchange_search_group.cpp
    instructions/general_arithmetic_group.cpp
    instructions/8bit_arithmetic_group.cpp
    instructions/16bit_arithmetic_group.cpp
    instructions/rotate_shift_group.cpp
    instructions/bit_set_reset_test_group.cpp
    instructions/jump_group.cpp
    instructions/call_return_group.cpp
    instructions/input_output_group.cpp
)

target_include_directories(Z80 PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}
)