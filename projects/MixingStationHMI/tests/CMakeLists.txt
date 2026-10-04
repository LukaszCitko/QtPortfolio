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

add_executable(test_eventmanager
    tst_eventmanager.cpp
    ../src/eventmanager.cpp
)

target_link_libraries(test_eventmanager
    PRIVATE Qt6::Test Qt6::Core
)
add_executable(test_trendrecorder
    tst_trendrecorder.cpp
    ../src/trendrecorder.cpp
    ../src/mixingtank.cpp
)

target_link_libraries(test_trendrecorder
    PRIVATE Qt6::Test Qt6::Core
)

add_executable(test_runrepository
    tst_runrepository.cpp
    ../src/database/databaseschema.cpp
    ../src/database/runrepository.cpp
)

target_link_libraries(test_runrepository
    PRIVATE Qt6::Test Qt6::Sql
)

add_executable(test_eventrepository
    tst_eventrepository.cpp
    ../src/database/databaseschema.cpp
    ../src/database/eventrepository.cpp
)

target_link_libraries(test_eventrepository
    PRIVATE Qt6::Test Qt6::Sql
)

add_executable(test_trendsamplerepository
    tst_trendsamplerepository.cpp
    ../src/database/databaseschema.cpp
    ../src/database/trendsamplerepository.cpp
)

target_link_libraries(test_trendsamplerepository
    PRIVATE Qt6::Test Qt6::Sql
)
add_executable(test_trendhistorysource
    tst_trendhistorysource.cpp
    ../src/database/databaseschema.cpp
    ../src/database/trendhistorysource.cpp
)

target_link_libraries(test_trendhistorysource
    PRIVATE Qt6::Test Qt6::Sql
)
add_executable(test_operatorsession
    tst_operatorsession.cpp
    ../src/database/databaseschema.cpp
    ../src/operatorsession.cpp
    ../src/accesspolicy.cpp
)

target_link_libraries(test_operatorsession
    PRIVATE Qt6::Test Qt6::Sql
)
add_executable(test_accesspolicy
    tst_accesspolicy.cpp
    ../src/accesspolicy.cpp
)

target_link_libraries(test_accesspolicy
    PRIVATE Qt6::Test Qt6::Core
)
add_executable(test_faultresetservice
    tst_faultresetservice.cpp
    ../src/database/databaseschema.cpp
    ../src/accesspolicy.cpp
    ../src/operatorsession.cpp
    ../src/pump.cpp
    ../src/mixer.cpp
    ../src/eventmanager.cpp
    ../src/faultresetservice.cpp
)

target_link_libraries(test_faultresetservice
    PRIVATE Qt6::Test Qt6::Sql
)
add_executable(test_mixer
    tst_mixer.cpp
    ../src/mixer.cpp
)

target_link_libraries(test_mixer
    PRIVATE Qt6::Test Qt6::Core
)
add_executable(test_drainservice
    tst_drainservice.cpp
    ../src/database/databaseschema.cpp
    ../src/accesspolicy.cpp
    ../src/operatorsession.cpp
    ../src/pump.cpp
    ../src/valve.cpp
    ../src/mixer.cpp
    ../src/mixingtank.cpp
    ../src/batchcontroller.cpp
    ../src/eventmanager.cpp
    ../src/drainservice.cpp
)

target_link_libraries(test_drainservice
    PRIVATE Qt6::Test Qt6::Sql
)
add_executable(test_userrepository
    tst_userrepository.cpp
    ../src/database/databaseschema.cpp
    ../src/database/userrepository.cpp
)

target_link_libraries(test_userrepository
    PRIVATE Qt6::Test Qt6::Sql
)
add_executable(test_usermanagementservice
    tst_usermanagementservice.cpp
    ../src/database/databaseschema.cpp
    ../src/database/userrepository.cpp
    ../src/database/userlistmodel.cpp
    ../src/operatorsession.cpp
    ../src/accesspolicy.cpp
    ../src/eventmanager.cpp
    ../src/usermanagementservice.cpp
)

target_link_libraries(test_usermanagementservice
    PRIVATE Qt6::Test Qt6::Sql
)
add_executable(test_demouserseeder
    tst_demouserseeder.cpp
    ../src/database/databaseschema.cpp
    ../src/database/demouserseeder.cpp
)

target_link_libraries(test_demouserseeder
    PRIVATE Qt6::Test Qt6::Sql
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
add_test(NAME test_mixingTank COMMAND test_mixingTank)
add_test(NAME test_batchcontroller COMMAND test_batchcontroller)