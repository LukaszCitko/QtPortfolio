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
    ../src/pump.cpp
    ../src/valve.cpp
    ../src/processsimulator.cpp
    ../src/mixingtank.cpp
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
    ../src/mixer.cpp
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

add_executable(test_mixingTank
    tst_mixingtank.cpp
    ../src/mixingtank.cpp
)
target_link_libraries(test_mixingTank
    PRIVATE
        Qt6::Test
        Qt6::Core
)

add_executable(test_batchcontroller
    tst_batchcontroller.cpp
    ../src/pump.cpp
    ../src/valve.cpp
    ../src/processsimulator.cpp
    ../src/mixingtank.cpp
    ../src/batchcontroller.cpp
    ../src/mixer.cpp

)

target_link_libraries(test_batchcontroller
    PRIVATE
        Qt6::Test
        Qt6::Quick
)


add_test(NAME test_processcontroller COMMAND test_processcontroller)
add_test(NAME test_processsimulator COMMAND test_processsimulator)
add_test(NAME test_pump COMMAND test_pump)
add_test(NAME test_valve COMMAND test_valve)
add_test(NAME test_mixingTank COMMAND test_mixingTank)
add_test(NAME test_batchcontroller COMMAND test_batchcontroller)