find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(test_pump
    tst_pump.cpp
    ../src/pump.cpp
)

target_link_libraries(test_pump
    PRIVATE Qt6::Test
)

add_executable(test_valve
    tst_valve.cpp
    ../src/valve.cpp
)

target_link_libraries(test_valve
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

add_executable(test_processcontroller
    tst_processcontroller.cpp
    ../src/pump.cpp
    ../src/pump.h
    ../src/valve.cpp
    ../src/valve.h
    ../src/processcontroller.cpp
    ../src/processcontroller.h
    ../src/eventmanager.h
    ../src/eventmanager.cpp
)

target_link_libraries(test_processcontroller
    PRIVATE
        Qt6::Test
        Qt6::Core
)

add_test(NAME test_processcontroller COMMAND test_processcontroller)
add_test(NAME test_processsimulator COMMAND test_processsimulator)
add_test(NAME test_pump COMMAND test_pump)
