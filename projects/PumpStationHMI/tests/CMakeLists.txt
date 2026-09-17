find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(test_pump
    tst_pump.cpp
    ../src/pump.cpp
)

target_link_libraries(test_pump
    PRIVATE Qt6::Test
)


add_executable(test_processsimulator
    tst_processsimulator.cpp
    ../src/Pump.cpp
    ../src/processsimulator.cpp
)

target_link_libraries(test_processsimulator
    PRIVATE
        Qt6::Test
        Qt6::Quick
)


add_test(NAME test_processsimulator COMMAND test_processsimulator)
add_test(NAME test_pump COMMAND test_pump)