cmake_minimum_required(VERSION 3.16)

project(MixingStationHMI VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_QML_GENERATE_QMLLS_INI ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Quick Sql SerialBus)

qt_standard_project_setup(REQUIRES 6.8)

function(enable_project_warnings target)
    if(MSVC)
        target_compile_options(${target}
            PRIVATE
                /W4
                /permissive-
        )
    else()
        target_compile_options(${target}
            PRIVATE
                -Wall
                -Wextra
                -Wpedantic
        )
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${target}
                PRIVATE
                    -Wno-gnu-zero-variadic-macro-arguments
            )
        endif()
    endif()

endfunction()

qt_add_library(mixingstation_core STATIC
    src/accesspolicy.cpp
    src/accesspolicy.h
    src/eventmanager.cpp
    src/eventmanager.h
    src/faultresetservice.cpp
    src/faultresetservice.h
    src/mixer.cpp
    src/mixer.h
    src/mixingtank.cpp
    src/mixingtank.h
    src/operatorsession.cpp
    src/operatorsession.h
    src/processsimulator.cpp
    src/processsimulator.h
    src/pump.cpp
    src/pump.h
    src/trendrecorder.cpp
    src/trendrecorder.h
    src/usermanagementservice.cpp
    src/usermanagementservice.h
    src/valve.cpp
    src/valve.h

    shared/controllerprotocol.h

    src/database/databaseschema.cpp
    src/database/databaseschema.h
    src/database/demouserseeder.cpp
    src/database/demouserseeder.h
    src/database/eventhistorymodel.cpp
    src/database/eventhistorymodel.h
    src/database/eventrepository.cpp
    src/database/eventrepository.h
    src/database/runlistmodel.cpp
    src/database/runlistmodel.h

    src/database/runrepository.cpp
    src/database/runrepository.h
    src/database/trendhistorysource.cpp
    src/database/trendhistorysource.h
    src/database/trendsamplerepository.cpp
    src/database/trendsamplerepository.h
    src/database/userlistmodel.cpp
    src/database/userlistmodel.h
    src/database/userrepository.cpp
    src/database/userrepository.h
)

target_include_directories(mixingstation_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/shared
)

target_link_libraries(mixingstation_core
    PUBLIC
        Qt6::Core
        Qt6::Sql
)

enable_project_warnings(mixingstation_core)

qt_add_executable(appControllerSimulator
    src/controllersimulator/main.cpp
)

set_target_properties(appControllerSimulator PROPERTIES
    MACOSX_BUNDLE FALSE
    WIN32_EXECUTABLE FALSE
)

target_include_directories(appControllerSimulator
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/shared
)

target_link_libraries(appControllerSimulator
    PRIVATE
        Qt6::Core
        Qt6::SerialBus
)

enable_project_warnings(appControllerSimulator)

qt_add_executable(appModbusClientProbe
    src/clientprobe/main.cpp
)

set_target_properties(appModbusClientProbe PROPERTIES
    MACOSX_BUNDLE FALSE
    WIN32_EXECUTABLE FALSE
)

target_include_directories(appModbusClientProbe
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/shared
)

target_link_libraries(appModbusClientProbe
    PRIVATE
        Qt6::Core
        Qt6::SerialBus
)

enable_project_warnings(appModbusClientProbe)


qt_add_executable(appMixingStationHMI
    src/main.cpp

    src/batchcontroller.cpp
    src/batchcontroller.h
    src/drainservice.cpp
    src/drainservice.h
    src/processeventcoordinator.cpp
    src/processeventcoordinator.h

    src/database/runpersistencecoordinator.cpp
    src/database/runpersistencecoordinator.h
)

qt_add_qml_module(appMixingStationHMI
    URI MixingStationHMI

    QML_FILES
        qml/AlarmBar.qml
        qml/BatchActions.qml
        qml/EquipmentLane.qml
        qml/EquipmentPanel.qml
        qml/HMIScreen.qml
        qml/PreCheckPanel.qml
        qml/ProcessValues.qml
        qml/SimulationControlPanel.qml
        qml/StageStrip.qml
        qml/TankView.qml
        qml/TrendChart.qml
        qml/TrendPanel.qml
        qml/UserManagementDialog.qml

    RESOURCES
        qml/assets/pump.png
        qml/assets/pump_gray.png
        qml/assets/valve.png
        qml/assets/valve_gray.png
)

target_include_directories(appMixingStationHMI
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(appMixingStationHMI PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.lukaszcitko.mixingstationhmi
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appMixingStationHMI
    PRIVATE
        mixingstation_core
        Qt6::Quick
        Qt6::Sql
)

enable_project_warnings(appMixingStationHMI)

install(TARGETS appMixingStationHMI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
