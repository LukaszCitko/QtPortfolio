find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(test_pump
    tst_pump.cpp
)

target_link_libraries(test_pump
    PRIVATE
        mixingstation_core
        Qt6::Test
)

add_executable(test_valve
    tst_valve.cpp
)

target_link_libraries(test_valve
    PRIVATE
        mixingstation_core
        Qt6::Test
)

add_executable(test_processsimulator
    tst_processsimulator.cpp

)

target_link_libraries(test_processsimulator
    PRIVATE
        mixingstation_core
        Qt6::Test
        Qt6::Core
)

add_executable(test_mixingtank
    tst_mixingtank.cpp
)

target_link_libraries(test_mixingtank
    PRIVATE
        mixingstation_core
        Qt6::Test
        Qt6::Core
)

add_executable(test_batchcontroller
    tst_batchcontroller.cpp
    ../src/batchcontroller.cpp
)

target_link_libraries(test_batchcontroller
    PRIVATE
        mixingstation_core
        Qt6::Test
        Qt6::Qml
)

add_executable(test_eventmanager
    tst_eventmanager.cpp
)

target_link_libraries(test_eventmanager
    PRIVATE
        mixingstation_core
        Qt6::Test
        Qt6::Core
)

add_executable(test_trendrecorder
    tst_trendrecorder.cpp
)

target_link_libraries(test_trendrecorder
    PRIVATE
        mixingstation_core
        Qt6::Test
        Qt6::Core
)

add_executable(test_runrepository
    tst_runrepository.cpp
)

target_link_libraries(test_runrepository
    PRIVATE
        mixingstation_core
        Qt6::Test
)

add_executable(test_eventrepository
    tst_eventrepository.cpp
)

target_link_libraries(test_eventrepository
    PRIVATE
        mixingstation_core
        Qt6::Test
)

add_executable(test_trendsamplerepository
    tst_trendsamplerepository.cpp
)

target_link_libraries(test_trendsamplerepository
    PRIVATE
        mixingstation_core
        Qt6::Test
)

add_executable(test_trendhistorysource
    tst_trendhistorysource.cpp
)

target_link_libraries(test_trendhistorysource
    PRIVATE
        mixingstation_core
        Qt6::Test
)

add_executable(test_userrepository
    tst_userrepository.cpp
)

target_link_libraries(test_userrepository
    PRIVATE
        mixingstation_core
        Qt6::Test
)

add_executable(test_operatorsession
    tst_operatorsession.cpp
)

target_link_libraries(test_operatorsession
    PRIVATE
        mixingstation_core
        Qt6::Test
)

add_executable(test_accesspolicy
    tst_accesspolicy.cpp
)

target_link_libraries(test_accesspolicy
    PRIVATE
        mixingstation_core
        Qt6::Test
)

add_executable(test_faultresetservice
    tst_faultresetservice.cpp
)

target_link_libraries(test_faultresetservice
    PRIVATE
        mixingstation_core
        Qt6::Test
)

add_executable(test_mixer
    tst_mixer.cpp
)

target_link_libraries(test_mixer
    PRIVATE
        mixingstation_core
        Qt6::Test
        Qt6::Core
)

add_executable(test_drainservice
    tst_drainservice.cpp
    ../src/batchcontroller.cpp
    ../src/drainservice.cpp
)

target_link_libraries(test_drainservice
    PRIVATE
        mixingstation_core
        Qt6::Test
        Qt6::Qml
)
add_executable(test_usermanagementservice
    tst_usermanagementservice.cpp
)

target_link_libraries(test_usermanagementservice
    PRIVATE
        mixingstation_core
        Qt6::Test
)

add_executable(test_demouserseeder
    tst_demouserseeder.cpp
)

target_link_libraries(test_demouserseeder
    PRIVATE
        mixingstation_core
        Qt6::Test
)

add_test(NAME test_demouserseeder COMMAND test_demouserseeder)
add_test(NAME test_usermanagementservice COMMAND test_usermanagementservice)
add_test(NAME test_userrepository COMMAND test_userrepository)
add_test(NAME test_drainservice COMMAND test_drainservice)
add_test(NAME test_mixer COMMAND test_mixer)
add_test(NAME test_faultresetservice COMMAND test_faultresetservice)
add_test(NAME test_accesspolicy COMMAND test_accesspolicy)
add_test(NAME test_operatorsession COMMAND test_operatorsession)
add_test(NAME test_trendhistorysource COMMAND test_trendhistorysource)
add_test(NAME test_trendsamplerepository COMMAND test_trendsamplerepository)
add_test(NAME test_eventrepository COMMAND test_eventrepository)
add_test(NAME test_runrepository COMMAND test_runrepository)
add_test(NAME test_trendrecorder COMMAND test_trendrecorder)
add_test(NAME test_eventmanager COMMAND test_eventmanager)
add_test(NAME test_processsimulator COMMAND test_processsimulator)
add_test(NAME test_pump COMMAND test_pump)
add_test(NAME test_valve COMMAND test_valve)
add_test(NAME test_mixingtank COMMAND test_mixingtank)
add_test(NAME test_batchcontroller COMMAND test_batchcontroller)
