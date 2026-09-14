find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(test_pump
    tst_pump.cpp
    ../src/pump.cpp
)

target_link_libraries(test_pump
    PRIVATE
        Qt6::Test
)

add_test(NAME test_pump COMMAND test_pump)